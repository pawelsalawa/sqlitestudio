/********************************************************
 * ADO.NET 2.0 Data Provider for SQLite Version 3.X
 * Written by Robert Simpson (robert@blackcastlesoft.com)
 *
 * Released to the public domain, use at your own risk!
 ********************************************************/

#ifndef SQLITE_OMIT_DISKIO
#ifdef SQLITE_HAS_CODEC

#include <windows.h>
#include <wincrypt.h>
#include "systemdata_sqlite3.h"

/* Extra padding before and after the cryptographic buffer */
#define CRYPT_OFFSET 8

typedef struct _CRYPTBLOCK
{
  Pager    *pPager;       /* Pager this cryptblock belongs to */
  HCRYPTKEY hReadKey;     /* Key used to read from the database and write to the journal */
  HCRYPTKEY hWriteKey;    /* Key used to write to the database */
  DWORD     dwPageSize;   /* Size of pages */
  LPVOID    pvCrypt;      /* A buffer for encrypting/decrypting (if necessary) */
  DWORD     dwCryptSize;  /* Equal to or greater than dwPageSize.  If larger, pvCrypt is valid and this is its size */
} CRYPTBLOCK, *LPCRYPTBLOCK;

HCRYPTPROV g_hProvider = 0; /* Global instance of the cryptographic provider */

#define SQLITECRYPTERROR_PROVIDER "Cryptographic provider not available"

/* Needed for re-keying */
static void * systemdata_sqlite3pager_get_codecarg(Pager *pPager)
{
  return (pPager->xCodec) ? pPager->pCodec: NULL;
}

void systemdata_sqlite3_activate_see(const char *info)
{
}

/* Create a cryptographic context.  Use the enhanced provider because it is available on
** most platforms
*/
static BOOL InitializeProvider()
{
  MUTEX_LOGIC( systemdata_sqlite3_mutex *pMaster = systemdata_sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER); )
  systemdata_sqlite3_mutex_enter(pMaster);

  if (g_hProvider)
  {
    systemdata_sqlite3_mutex_leave(pMaster);
    return TRUE;
  }

  if (!CryptAcquireContext(&g_hProvider, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
  {
    systemdata_sqlite3_mutex_leave(pMaster);
    return FALSE;
  }

  systemdata_sqlite3_mutex_leave(pMaster);
  return TRUE;
}

/* Create or update a cryptographic context for a pager.
** This function will automatically determine if the encryption algorithm requires
** extra padding, and if it does, will create a temp buffer big enough to provide
** space to hold it.
*/
static LPCRYPTBLOCK CreateCryptBlock(HCRYPTKEY hKey, Pager *pager, int pageSize, LPCRYPTBLOCK pExisting)
{
  LPCRYPTBLOCK pBlock;

  if (!pExisting) /* Creating a new cryptblock */
  {
    pBlock = systemdata_sqlite3_malloc(sizeof(CRYPTBLOCK));
    if (!pBlock) return NULL;

    ZeroMemory(pBlock, sizeof(CRYPTBLOCK));
    pBlock->hReadKey = hKey;
    pBlock->hWriteKey = hKey;
  }
  else /* Updating an existing cryptblock */
  {
    pBlock = pExisting;
  }

  if (pageSize == -1)
    pageSize = pager->pageSize;

  pBlock->pPager = pager;
  pBlock->dwPageSize = (DWORD)pageSize;
  pBlock->dwCryptSize = pBlock->dwPageSize;

  /* Existing cryptblocks may have a buffer, if so, delete it */
  if (pBlock->pvCrypt)
  {
    systemdata_sqlite3_free(pBlock->pvCrypt);
    pBlock->pvCrypt = NULL;
  }

  /* Figure out how big to make our spare crypt block */
  CryptEncrypt(hKey, 0, TRUE, 0, NULL, &pBlock->dwCryptSize, pBlock->dwCryptSize * 2);
  pBlock->pvCrypt = systemdata_sqlite3_malloc(pBlock->dwCryptSize + (CRYPT_OFFSET * 2));
  if (!pBlock->pvCrypt)
  {
    /* We created a new block in here, so free it.  Otherwise leave the original intact */
    if (pBlock != pExisting)
      systemdata_sqlite3_free(pBlock);

    return NULL;
  }

  return pBlock;
}

/* Destroy a cryptographic context and any buffers and keys allocated therein */
static void systemdata_sqlite3CodecFree(LPVOID pv)
{
  LPCRYPTBLOCK pBlock = (LPCRYPTBLOCK)pv;
  /* Destroy the read key if there is one */
  if (pBlock->hReadKey)
  {
    CryptDestroyKey(pBlock->hReadKey);
  }

  /* If there's a writekey and its not equal to the readkey, destroy it */
  if (pBlock->hWriteKey && pBlock->hWriteKey != pBlock->hReadKey)
  {
    CryptDestroyKey(pBlock->hWriteKey);
  }

  /* If there's extra buffer space allocated, free it as well */
  if (pBlock->pvCrypt)
  {
    systemdata_sqlite3_free(pBlock->pvCrypt);
  }

  /* All done with this cryptblock */
  systemdata_sqlite3_free(pBlock);
}

void systemdata_sqlite3CodecSizeChange(void *pArg, int pageSize, int reservedSize)
{
  LPCRYPTBLOCK pBlock = (LPCRYPTBLOCK)pArg;

  if (pBlock->dwPageSize != pageSize)
  {
    CreateCryptBlock(pBlock->hReadKey, pBlock->pPager, pageSize, pBlock);
    /* If this fails, pvCrypt will be NULL, and the next time systemdata_sqlite3Codec() is called, it will result in an error */
  }
}

/* Encrypt/Decrypt functionality, called by pager.c */
void * systemdata_sqlite3Codec(void *pArg, void *data, Pgno nPageNum, int nMode)
{
  LPCRYPTBLOCK pBlock = (LPCRYPTBLOCK)pArg;
  DWORD dwPageSize;
  LPVOID pvTemp = NULL;

  if (!pBlock) return data;
  if (pBlock->pvCrypt == NULL) return NULL; /* This only happens if CreateCryptBlock() failed to make scratch space */

  switch(nMode)
  {
  case 0: /* Undo a "case 7" journal file encryption */
  case 2: /* Reload a page */
  case 3: /* Load a page */
    if (!pBlock->hReadKey) break;

    /* Block ciphers often need to write extra padding beyond the
    data block.  We don't have that luxury for a given page of data so
    we must copy the page data to a buffer that IS large enough to hold
    the padding.  We then encrypt the block and write the buffer back to
    the page without the unnecessary padding.
    We only use the special block of memory if its absolutely necessary. */
    if (pBlock->dwCryptSize != pBlock->dwPageSize)
    {
      CopyMemory(((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET, data, pBlock->dwPageSize);
      pvTemp = data;
      data = ((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET;
    }

    dwPageSize = pBlock->dwCryptSize;
    CryptDecrypt(pBlock->hReadKey, 0, TRUE, 0, (LPBYTE)data, &dwPageSize);

    /* If the encryption algorithm required extra padding and we were forced to encrypt or
    ** decrypt a copy of the page data to a temp buffer, then write the contents of the temp
    ** buffer back to the page data minus any padding applied.
    */
    if (pBlock->dwCryptSize != pBlock->dwPageSize)
    {
      CopyMemory(pvTemp, data, pBlock->dwPageSize);
      data = pvTemp;
    }
    break;
  case 6: /* Encrypt a page for the main database file */
    if (!pBlock->hWriteKey) break;

    CopyMemory(((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET, data, pBlock->dwPageSize);
    data = ((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET;

    dwPageSize = pBlock->dwPageSize;
    CryptEncrypt(pBlock->hWriteKey, 0, TRUE, 0, ((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET, &dwPageSize, pBlock->dwCryptSize);
    break;
  case 7: /* Encrypt a page for the journal file */
    /* Under normal circumstances, the readkey is the same as the writekey.  However,
    when the database is being rekeyed, the readkey is not the same as the writekey.
    The rollback journal must be written using the original key for the
    database file because it is, by nature, a rollback journal.
    Therefore, for case 7, when the rollback is being written, always encrypt using
    the database's readkey, which is guaranteed to be the same key that was used to
    read the original data.
    */
    if (!pBlock->hReadKey) break;

    CopyMemory(((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET, data, pBlock->dwPageSize);
    data = ((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET;

    dwPageSize = pBlock->dwPageSize;
    CryptEncrypt(pBlock->hReadKey, 0, TRUE, 0, ((LPBYTE)pBlock->pvCrypt) + CRYPT_OFFSET, &dwPageSize, pBlock->dwCryptSize);
    break;
  }

  return data;
}

/* Derive an encryption key from a user-supplied buffer */
static HCRYPTKEY DeriveKey(const void *pKey, int nKey)
{
  HCRYPTHASH hHash = 0;
  HCRYPTKEY  hKey;

  if (!pKey || !nKey) return 0;

  if (!InitializeProvider())
  {
    return MAXDWORD;
  }

  {
    MUTEX_LOGIC( systemdata_sqlite3_mutex *pMaster = systemdata_sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER); )
    systemdata_sqlite3_mutex_enter(pMaster);

    if (CryptCreateHash(g_hProvider, CALG_SHA1, 0, 0, &hHash))
    {
      if (CryptHashData(hHash, (LPBYTE)pKey, nKey, 0))
      {
        CryptDeriveKey(g_hProvider, CALG_RC4, hHash, 0, &hKey);
      }
      CryptDestroyHash(hHash);
    }

    systemdata_sqlite3_mutex_leave(pMaster);
  }

  return hKey;
}

/* Called by sqlite and systemdata_sqlite3_key_interop to attach a key to a database. */
int systemdata_sqlite3CodecAttach(systemdata_sqlite3 *db, int nDb, const void *pKey, int nKeyLen)
{
  int rc = SQLITE_ERROR;
  HCRYPTKEY hKey = 0;

  /* No key specified, could mean either use the main db's encryption or no encryption */
  if (!pKey || !nKeyLen)
  {
    if (!nDb)
    {
      return SQLITE_OK; /* Main database, no key specified so not encrypted */
    }
    else /* Attached database, use the main database's key */
    {
      /* Get the encryption block for the main database and attempt to duplicate the key
      ** for use by the attached database
      */
      Pager *p = systemdata_sqlite3BtreePager(db->aDb[0].pBt);
      LPCRYPTBLOCK pBlock = (LPCRYPTBLOCK)systemdata_sqlite3pager_get_codecarg(p);

      if (!pBlock) return SQLITE_OK; /* Main database is not encrypted so neither will be any attached database */
      if (!pBlock->hReadKey) return SQLITE_OK; /* Not encrypted */

      if (!CryptDuplicateKey(pBlock->hReadKey, NULL, 0, &hKey))
        return rc; /* Unable to duplicate the key */
    }
  }
  else /* User-supplied passphrase, so create a cryptographic key out of it */
  {
    hKey = DeriveKey(pKey, nKeyLen);
    if (hKey == MAXDWORD)
    {
#if SQLITE_VERSION_NUMBER >= 3008007
      systemdata_sqlite3ErrorWithMsg(db, rc, SQLITECRYPTERROR_PROVIDER);
#else
      systemdata_sqlite3Error(db, rc, SQLITECRYPTERROR_PROVIDER);
#endif
      return rc;
    }
  }

  /* Create a new encryption block and assign the codec to the new attached database */
  if (hKey)
  {
    Pager *p = systemdata_sqlite3BtreePager(db->aDb[nDb].pBt);
    LPCRYPTBLOCK pBlock = CreateCryptBlock(hKey, p, -1, NULL);
    if (!pBlock) return SQLITE_NOMEM;

    systemdata_sqlite3PagerSetCodec(p, systemdata_sqlite3Codec, systemdata_sqlite3CodecSizeChange, systemdata_sqlite3CodecFree, pBlock);

    rc = SQLITE_OK;
  }
  return rc;
}

/* Once a password has been supplied and a key created, we don't keep the
** original password for security purposes.  Therefore return NULL.
*/
void systemdata_sqlite3CodecGetKey(systemdata_sqlite3 *db, int nDb, void **ppKey, int *pnKeyLen)
{
  Btree *pbt = db->aDb[0].pBt;
  Pager *p = systemdata_sqlite3BtreePager(pbt);
  LPCRYPTBLOCK pBlock = (LPCRYPTBLOCK)systemdata_sqlite3pager_get_codecarg(p);

  if (ppKey) *ppKey = 0;
  if (pnKeyLen) *pnKeyLen = pBlock ? 1: 0;
}

/* We do not attach this key to the temp store, only the main database. */
SQLITE_API int systemdata_sqlite3_key_v2(systemdata_sqlite3 *db, const char *zDbName, const void *pKey, int nKey)
{
  return systemdata_sqlite3CodecAttach(db, 0, pKey, nKey);
}

SQLITE_API int systemdata_sqlite3_key(systemdata_sqlite3 *db, const void *pKey, int nKey)
{
  return systemdata_sqlite3_key_v2(db, 0, pKey, nKey);
}

/* Changes the encryption key for an existing database. */
SQLITE_API int systemdata_sqlite3_rekey_v2(systemdata_sqlite3 *db, const char *zDbName, const void *pKey, int nKey)
{
  Btree *pbt = db->aDb[0].pBt;
  Pager *p = systemdata_sqlite3BtreePager(pbt);
  LPCRYPTBLOCK pBlock = (LPCRYPTBLOCK)systemdata_sqlite3pager_get_codecarg(p);
  HCRYPTKEY hKey = DeriveKey(pKey, nKey);
  int rc = SQLITE_ERROR;

  if (hKey == MAXDWORD)
  {
#if SQLITE_VERSION_NUMBER >= 3008007
    systemdata_sqlite3ErrorWithMsg(db, rc, SQLITECRYPTERROR_PROVIDER);
#else
    systemdata_sqlite3Error(db, rc, SQLITECRYPTERROR_PROVIDER);
#endif
    return rc;
  }

  if (!pBlock && !hKey) return SQLITE_OK; /* Wasn't encrypted to begin with */

  /* To rekey a database, we change the writekey for the pager.  The readkey remains
  ** the same
  */
  if (!pBlock) /* Encrypt an unencrypted database */
  {
    pBlock = CreateCryptBlock(hKey, p, -1, NULL);
    if (!pBlock)
      return SQLITE_NOMEM;

    pBlock->hReadKey = 0; /* Original database is not encrypted */
    systemdata_sqlite3PagerSetCodec(systemdata_sqlite3BtreePager(pbt), systemdata_sqlite3Codec, systemdata_sqlite3CodecSizeChange, systemdata_sqlite3CodecFree, pBlock);
  }
  else /* Change the writekey for an already-encrypted database */
  {
    pBlock->hWriteKey = hKey;
  }

  systemdata_sqlite3_mutex_enter(db->mutex);

  /* Start a transaction */
  rc = systemdata_sqlite3BtreeBeginTrans(pbt, 1);

  if (!rc)
  {
    /* Rewrite all the pages in the database using the new encryption key */
    Pgno nPage;
    Pgno nSkip = PAGER_MJ_PGNO(p);
    DbPage *pPage;
    Pgno n;
    int count;

    systemdata_sqlite3PagerPagecount(p, &count);
    nPage = (Pgno)count;

    for(n = 1; n <= nPage; n ++)
    {
      if (n == nSkip) continue;
      rc = INTEROP_CODEC_GET_PAGER(p, n, &pPage);
      if(!rc)
      {
        rc = systemdata_sqlite3PagerWrite(pPage);
        systemdata_sqlite3PagerUnref(pPage);
      }
    }
  }

  /* If we succeeded, try and commit the transaction */
  if (!rc)
  {
    rc = systemdata_sqlite3BtreeCommit(pbt);
  }

  // If we failed, rollback */
  if (rc)
  {
#if SQLITE_VERSION_NUMBER >= 3008007
    systemdata_sqlite3BtreeRollback(pbt, SQLITE_OK, 0);
#else
    systemdata_sqlite3BtreeRollback(pbt, SQLITE_OK);
#endif
  }

  /* If we succeeded, destroy any previous read key this database used
  ** and make the readkey equal to the writekey
  */
  if (!rc)
  {
    if (pBlock->hReadKey)
    {
      CryptDestroyKey(pBlock->hReadKey);
    }
    pBlock->hReadKey = pBlock->hWriteKey;
  }
  /* We failed.  Destroy the new writekey (if there was one) and revert it back to
  ** the original readkey
  */
  else
  {
    if (pBlock->hWriteKey)
    {
      CryptDestroyKey(pBlock->hWriteKey);
    }
    pBlock->hWriteKey = pBlock->hReadKey;
  }

  /* If the readkey and writekey are both empty, there's no need for a codec on this
  ** pager anymore.  Destroy the crypt block and remove the codec from the pager.
  */
  if (!pBlock->hReadKey && !pBlock->hWriteKey)
  {
    systemdata_sqlite3PagerSetCodec(p, NULL, NULL, NULL, NULL);
  }

  systemdata_sqlite3_mutex_leave(db->mutex);

  return rc;
}

SQLITE_API int systemdata_sqlite3_rekey(systemdata_sqlite3 *db, const void *pKey, int nKey)
{
  return systemdata_sqlite3_rekey_v2(db, 0, pKey, nKey);
}

#endif /* SQLITE_HAS_CODEC */
#endif /* SQLITE_OMIT_DISKIO */








