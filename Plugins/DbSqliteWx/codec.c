/*
** Name:        codec.c
** Purpose:     Implementation of SQLite codecs
** Author:      Ulrich Telle
** Created:     2006-12-06
** Copyright:   (c) 2006-2018 Ulrich Telle
** License:     LGPL-3.0+ WITH WxWindows-exception-3.1
*/

#include "codec.h"

/*
** RC4 implementation
*/

void
CodecRC4(unsigned char* key, int keylen,
         unsigned char* textin, int textlen,
         unsigned char* textout)
{
  int i;
  int j;
  int t;
  unsigned char rc4[256];

  int a = 0;
  int b = 0;
  unsigned char k;

  for (i = 0; i < 256; i++)
  {
    rc4[i] = i;
  }
  j = 0;
  for (i = 0; i < 256; i++)
  {
    t = rc4[i];
    j = (j + t + key[i % keylen]) % 256;
    rc4[i] = rc4[j];
    rc4[j] = t;
  }

  for (i = 0; i < textlen; i++)
  {
    a = (a + 1) % 256;
    t = rc4[a];
    b = (b + t) % 256;
    rc4[a] = rc4[b];
    rc4[b] = t;
    k = rc4[(rc4[a] + rc4[b]) % 256];
    textout[i] = textin[i] ^ k;
  }
}

void
CodecGetMD5Binary(unsigned char* data, int length, unsigned char* digest)
{
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, data, length);
  MD5_Final(digest,&ctx);
}

void
CodecGetSHABinary(unsigned char* data, int length, unsigned char* digest)
{
  sha256(data, (unsigned int) length, digest);
}

#define MODMULT(a, b, c, m, s) q = s / a; s = b * (s - a * q) - c * q; if (s < 0) s += m

void
CodecGenerateInitialVector(int seed, unsigned char iv[16])
{
  unsigned char initkey[16];
  int j, q;
  int z = seed + 1;
  for (j = 0; j < 4; j++)
  {
    MODMULT(52774, 40692,  3791, 2147483399L, z);
    initkey[4*j+0] = 0xff &  z;
    initkey[4*j+1] = 0xff & (z >>  8);
    initkey[4*j+2] = 0xff & (z >> 16);
    initkey[4*j+3] = 0xff & (z >> 24);
  }
  CodecGetMD5Binary((unsigned char*) initkey, 16, iv);
}

int
CodecAES128(Rijndael* aesCtx, int page, int encrypt, unsigned char encryptionKey[KEYLENGTH_AES128],
            unsigned char* datain, int datalen, unsigned char* dataout)
{
  int rc = SQLITE_OK;
  unsigned char initial[16];
  unsigned char pagekey[KEYLENGTH_AES128];
  unsigned char nkey[KEYLENGTH_AES128+4+4];
  int keyLength = KEYLENGTH_AES128;
  int nkeylen = keyLength + 4 + 4;
  int j;
  int direction = (encrypt) ? RIJNDAEL_Direction_Encrypt : RIJNDAEL_Direction_Decrypt;
  int len = 0;

  for (j = 0; j < keyLength; j++)
  {
    nkey[j] = encryptionKey[j];
  }
  nkey[keyLength+0] = 0xff &  page;
  nkey[keyLength+1] = 0xff & (page >>  8);
  nkey[keyLength+2] = 0xff & (page >> 16);
  nkey[keyLength+3] = 0xff & (page >> 24);

  /* AES encryption needs some 'salt' */
  nkey[keyLength+4] = 0x73;
  nkey[keyLength+5] = 0x41;
  nkey[keyLength+6] = 0x6c;
  nkey[keyLength+7] = 0x54;

  CodecGetMD5Binary(nkey, nkeylen, pagekey);
  CodecGenerateInitialVector(page, initial);
  RijndaelInit(aesCtx, RIJNDAEL_Direction_Mode_CBC, direction, pagekey, RIJNDAEL_Direction_KeyLength_Key16Bytes, initial);
  if (encrypt)
  {
    len = RijndaelBlockEncrypt(aesCtx, datain, datalen*8, dataout);
  }
  else
  {
    len = RijndaelBlockDecrypt(aesCtx, datain, datalen*8, dataout);
  }
  
  /* It is a good idea to check the error code */
  if (len < 0)
  {
    /* AES: Error on encrypting. */
    rc = SQLITE_ERROR;
  }
  return rc;
}

int
CodecAES256(Rijndael* aesCtx, int page, int encrypt, unsigned char encryptionKey[KEYLENGTH_AES256],
            unsigned char* datain, int datalen, unsigned char* dataout)
{
  int rc = SQLITE_OK;
  unsigned char initial[16];
  unsigned char pagekey[KEYLENGTH_AES256];
  unsigned char nkey[KEYLENGTH_AES256+4+4];
  int keyLength = KEYLENGTH_AES256;
  int nkeylen = keyLength + 4 + 4;
  int j;
  int direction = (encrypt) ? RIJNDAEL_Direction_Encrypt : RIJNDAEL_Direction_Decrypt;
  int len = 0;

  for (j = 0; j < keyLength; j++)
  {
    nkey[j] = encryptionKey[j];
  }
  nkey[keyLength+0] = 0xff &  page;
  nkey[keyLength+1] = 0xff & (page >>  8);
  nkey[keyLength+2] = 0xff & (page >> 16);
  nkey[keyLength+3] = 0xff & (page >> 24);

  /* AES encryption needs some 'salt' */
  nkey[keyLength+4] = 0x73;
  nkey[keyLength+5] = 0x41;
  nkey[keyLength+6] = 0x6c;
  nkey[keyLength+7] = 0x54;

  CodecGetSHABinary(nkey, nkeylen, pagekey);
  CodecGenerateInitialVector(page, initial);
  RijndaelInit(aesCtx, RIJNDAEL_Direction_Mode_CBC, direction, pagekey, RIJNDAEL_Direction_KeyLength_Key32Bytes, initial);
  if (encrypt)
  {
    len = RijndaelBlockEncrypt(aesCtx, datain, datalen*8, dataout);
  }
  else
  {
    len = RijndaelBlockDecrypt(aesCtx, datain, datalen*8, dataout);
  }
  
  /* It is a good idea to check the error code */
  if (len < 0)
  {
    /* AES: Error on encrypting. */
    rc = SQLITE_ERROR;
  }
  return rc;
}

static unsigned char padding[] =
  "\x28\xBF\x4E\x5E\x4E\x75\x8A\x41\x64\x00\x4E\x56\xFF\xFA\x01\x08\x2E\x2E\x00\xB6\xD0\x68\x3E\x80\x2F\x0C\xA9\xFE\x64\x53\x69\x7A";

/* --- Codec Descriptor Table --- */

#define CIPHER_PARAMS_SENTINEL  { "", 0, 0, 0, 0 }
#define CIPHER_PAGE1_OFFSET 24

typedef struct _CipherParams
{
  char* m_name;
  int   m_value;
  int   m_default;
  int   m_minValue;
  int   m_maxValue;
} CipherParams;

/*
** Common configuration parameters
**
** - cipher : default cipher type
*/

CipherParams commonParams[] =
{
  { "cipher", CODEC_TYPE, CODEC_TYPE, 1, CODEC_TYPE_MAX },
  CIPHER_PARAMS_SENTINEL
};

/*
** Configuration parameters for "aes128cbc"
**
** - legacy mode : compatibility with first version (page 1 encrypted)
**                 possible values:  1 = yes, 0 = no (default)
*/

#ifdef WXSQLITE3_USE_OLD_ENCRYPTION_SCHEME
#define AES128_LEGACY_DEFAULT 1
#else
#define AES128_LEGACY_DEFAULT 0
#endif

CipherParams aes128Params[] =
{
  { "legacy",            AES128_LEGACY_DEFAULT, AES128_LEGACY_DEFAULT, 0, 1 },
  { "legacy_page_size",  0,                     0,                     0, SQLITE_MAX_PAGE_SIZE },
  CIPHER_PARAMS_SENTINEL
};

/*
** Configuration parameters for "aes256cbc"
**
** - legacy mode : compatibility with first version (page 1 encrypted)
**                 possible values:  1 = yes, 0 = no (default)
** - kdf_iter : number of iterations for key derivation
*/

#ifdef WXSQLITE3_USE_OLD_ENCRYPTION_SCHEME
#define AES256_LEGACY_DEFAULT 1
#else
#define AES256_LEGACY_DEFAULT 0
#endif

CipherParams aes256Params[] =
{
  { "legacy",            AES256_LEGACY_DEFAULT, AES256_LEGACY_DEFAULT, 0, 1 },
  { "legacy_page_size",  0,                     0,                     0, SQLITE_MAX_PAGE_SIZE },
  { "kdf_iter",          CODEC_SHA_ITER,        CODEC_SHA_ITER,        1, 0x7fffffff },
  CIPHER_PARAMS_SENTINEL
};

/*
** Configuration parameters for "chacha20"
**
** - legacy mode : compatibility with original sqleet
**                 (page 1 encrypted, kdf_iter = 12345)
**                 possible values:  1 = yes, 0 = no
** - kdf_iter : number of iterations for key derivation
*/

#ifdef WXSQLITE3_USE_SQLEET_LEGACY
#define CHACHA20_LEGACY_DEFAULT   1
#else
#define CHACHA20_LEGACY_DEFAULT   0
#endif

#define CHACHA20_KDF_ITER_DEFAULT 64007
#define SQLEET_KDF_ITER           12345
#define CHACHA20_LEGACY_PAGE_SIZE 4096

CipherParams chacha20Params[] =
{
  { "legacy",            CHACHA20_LEGACY_DEFAULT,   CHACHA20_LEGACY_DEFAULT,   0, 1 },
  { "legacy_page_size",  CHACHA20_LEGACY_PAGE_SIZE, CHACHA20_LEGACY_PAGE_SIZE, 0, SQLITE_MAX_PAGE_SIZE },
  { "kdf_iter",          CHACHA20_KDF_ITER_DEFAULT, CHACHA20_KDF_ITER_DEFAULT, 1, 0x7fffffff },
  CIPHER_PARAMS_SENTINEL
};

/*
** Configuration parameters for "sqlcipher"
**
** - kdf_iter        : number of iterations for key derivation
** - fast_kdf_iter   : number of iterations for hmac key
** - hmac_use        : flag whether to use hmac
** - hmac_pgno       : storage type for page number in hmac (native, le, be)
** - hmac_salt_mask  : mask byte for hmac salt
*/

#ifdef WXSQLITE3_USE_SQLCIPHER_LEGACY
#define SQLCIPHER_LEGACY_DEFAULT   1
#else
#define SQLCIPHER_LEGACY_DEFAULT   0
#endif

#define SQLCIPHER_KDF_ITER          64000
#define SQLCIPHER_FAST_KDF_ITER     2
#define SQLCIPHER_HMAC_USE          1
#define SQLCIPHER_HMAC_PGNO_LE      1
#define SQLCIPHER_HMAC_PGNO_BE      2
#define SQLCIPHER_HMAC_PGNO_NATIVE  0
#define SQLCIPHER_HMAC_SALT_MASK    0x3a
#define SQLCIPHER_LEGACY_PAGE_SIZE  1024

CipherParams sqlCipherParams[] =
{
  { "legacy",            SQLCIPHER_LEGACY_DEFAULT,   SQLCIPHER_LEGACY_DEFAULT,   0, 1 },
  { "legacy_page_size",  SQLCIPHER_LEGACY_PAGE_SIZE, SQLCIPHER_LEGACY_PAGE_SIZE, 0, SQLITE_MAX_PAGE_SIZE },
  { "kdf_iter",          SQLCIPHER_KDF_ITER,         SQLCIPHER_KDF_ITER,         1, 0x7fffffff },
  { "fast_kdf_iter",     SQLCIPHER_FAST_KDF_ITER,    SQLCIPHER_FAST_KDF_ITER,    1, 0x7fffffff },
  { "hmac_use",          SQLCIPHER_HMAC_USE,         SQLCIPHER_HMAC_USE,         0, 1 },
  { "hmac_pgno",         SQLCIPHER_HMAC_PGNO_LE,     SQLCIPHER_HMAC_PGNO_LE,     0, 2 },
  { "hmac_salt_mask",    SQLCIPHER_HMAC_SALT_MASK,   SQLCIPHER_HMAC_SALT_MASK,   0x00, 0xff },
  CIPHER_PARAMS_SENTINEL
};

int
GetCipherParameter(CipherParams* cipherParams, const char* paramName)
{
  int value = -1;
  for (; strlen(cipherParams->m_name) > 0; ++cipherParams)
  {
    if (wx_sqlite3_stricmp(paramName, cipherParams->m_name) == 0) break;
  }
  if (strlen(cipherParams->m_name) > 0)
  {
    value = cipherParams->m_value;
    cipherParams->m_value = cipherParams->m_default;
  }
  return value;
}

/* --- AES 128-bit cipher (wxSQLite3) --- */

typedef struct _AES128Cipher
{
  int           m_legacy;
  int           m_legacyPageSize;
  int           m_keyLength;
  unsigned char m_key[KEYLENGTH_AES128];
  Rijndael*     m_aes;
} AES128Cipher;

void*
AllocateAES128Cipher(wx_sqlite3* db)
{
  AES128Cipher* aesCipher = (AES128Cipher*) wx_sqlite3_malloc(sizeof(AES128Cipher));
  if (aesCipher != NULL)
  {
    aesCipher->m_aes = (Rijndael*) wx_sqlite3_malloc(sizeof(Rijndael));
    if (aesCipher->m_aes != NULL)
    {
      aesCipher->m_keyLength = KEYLENGTH_AES128;
      memset(aesCipher->m_key, 0, KEYLENGTH_AES128);
      RijndaelCreate(aesCipher->m_aes);
    }
    else
    {
      wx_sqlite3_free(aesCipher);
      aesCipher = NULL;
    }
  }
  if (aesCipher != NULL)
  {
    CipherParams* cipherParams = (CipherParams*) GetCipherParams(db, CODEC_TYPE_AES128);
    aesCipher->m_legacy = GetCipherParameter(cipherParams, "legacy");
    aesCipher->m_legacyPageSize = GetCipherParameter(cipherParams, "legacy_page_size");
  }
  return aesCipher;
}

void
FreeAES128Cipher(void* cipher)
{
  AES128Cipher* localCipher = (AES128Cipher*) cipher;
  memset(localCipher->m_aes, 0, sizeof(Rijndael));
  wx_sqlite3_free(localCipher->m_aes);
  memset(localCipher, 0, sizeof(AES128Cipher));
  wx_sqlite3_free(localCipher);
}

void
CloneAES128Cipher(void* cipherTo, void* cipherFrom)
{
  AES128Cipher* aesCipherTo = (AES128Cipher*) cipherTo;
  AES128Cipher* aesCipherFrom = (AES128Cipher*) cipherFrom;
  aesCipherTo->m_legacy = aesCipherFrom->m_legacy;
  aesCipherTo->m_legacyPageSize = aesCipherFrom->m_legacyPageSize;
  aesCipherTo->m_keyLength = aesCipherFrom->m_keyLength;
  memcpy(aesCipherTo->m_key, aesCipherFrom->m_key, KEYLENGTH_AES128);
  RijndaelInvalidate(aesCipherTo->m_aes);
  RijndaelInvalidate(aesCipherFrom->m_aes);
}

int
GetLegacyAES128Cipher(void* cipher)
{
  AES128Cipher* aesCipher = (AES128Cipher*)cipher;
  return aesCipher->m_legacy;
}

int
GetPageSizeAES128Cipher(void* cipher)
{
  AES128Cipher* aesCipher = (AES128Cipher*) cipher;
  int pageSize = 0;
  if (aesCipher->m_legacy != 0)
  {
    pageSize = aesCipher->m_legacyPageSize;
    if ((pageSize < 512) || (pageSize > SQLITE_MAX_PAGE_SIZE) || (((pageSize - 1) & pageSize) != 0))
    {
      pageSize = 0;
    }
  }
  return pageSize;
}

int
GetReservedAES128Cipher(void* cipher)
{
  return 0;
}

void
GenerateKeyAES128Cipher(void* cipher, Btree* pBt, char* userPassword, int passwordLength, int rekey)
{
  AES128Cipher* aesCipher = (AES128Cipher*) cipher;
  unsigned char userPad[32];
  unsigned char ownerPad[32];
  unsigned char ownerKey[32];

  unsigned char mkey[MD5_HASHBYTES];
  unsigned char digest[MD5_HASHBYTES];
  int keyLength = MD5_HASHBYTES;
  int i, j, k;
  MD5_CTX ctx;

  /* Pad passwords */
  CodecPadPassword(userPassword, passwordLength, userPad);
  CodecPadPassword("", 0, ownerPad);

  /* Compute owner key */

  MD5_Init(&ctx);
  MD5_Update(&ctx, ownerPad, 32);
  MD5_Final(digest, &ctx);

  /* only use for the input as many bit as the key consists of */
  for (k = 0; k < 50; ++k)
  {
    MD5_Init(&ctx);
    MD5_Update(&ctx, digest, keyLength);
    MD5_Final(digest, &ctx);
  }
  memcpy(ownerKey, userPad, 32);
  for (i = 0; i < 20; ++i)
  {
    for (j = 0; j < keyLength; ++j)
    {
      mkey[j] = (digest[j] ^ i);
    }
    CodecRC4(mkey, keyLength, ownerKey, 32, ownerKey);
  }

  /* Compute encryption key */

  MD5_Init(&ctx);
  MD5_Update(&ctx, userPad, 32);
  MD5_Update(&ctx, ownerKey, 32);
  MD5_Final(digest, &ctx);

  /* only use the really needed bits as input for the hash */
  for (k = 0; k < 50; ++k)
  {
    MD5_Init(&ctx);
    MD5_Update(&ctx, digest, keyLength);
    MD5_Final(digest, &ctx);
  }
  memcpy(aesCipher->m_key, digest, aesCipher->m_keyLength);
}

int
EncryptPageAES128Cipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  AES128Cipher* aesCipher = (AES128Cipher*) cipher;
  int rc = SQLITE_OK;
  if (aesCipher->m_legacy != 0)
  {
    /* Use the legacy encryption scheme */
    unsigned char* key = aesCipher->m_key;
    rc = CodecAES128(aesCipher->m_aes, page, 1, key, data, len, data);
  }
  else
  {
    unsigned char dbHeader[8];
    int offset = 0;
    unsigned char* key = aesCipher->m_key;
    if (page == 1)
    {
      /* Save the header bytes remaining unencrypted */
      memcpy(dbHeader, data + 16, 8);
      offset = 16;
      CodecAES128(aesCipher->m_aes, page, 1, key, data, 16, data);
    }
    rc = CodecAES128(aesCipher->m_aes, page, 1, key, data + offset, len - offset, data + offset);
    if (page == 1)
    {
      /* Move the encrypted header bytes 16..23 to a safe position */
      memcpy(data + 8, data + 16, 8);
      /* Restore the unencrypted header bytes 16..23 */
      memcpy(data + 16, dbHeader, 8);
    }
  }
  return rc;
}

int
DecryptPageAES128Cipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  AES128Cipher* aesCipher = (AES128Cipher*) cipher;
  int rc = SQLITE_OK;
  if (aesCipher->m_legacy != 0)
  {
    /* Use the legacy encryption scheme */
    rc = CodecAES128(aesCipher->m_aes, page, 0, aesCipher->m_key, data, len, data);
  }
  else
  {
    unsigned char dbHeader[8];
    int dbPageSize;
    int offset = 0;
    if (page == 1)
    {
      /* Save (unencrypted) header bytes 16..23 */
      memcpy(dbHeader, data + 16, 8);
      /* Determine page size */
      dbPageSize = (dbHeader[0] << 8) | (dbHeader[1] << 16);
      /* Check whether the database header is valid */
      /* If yes, the database follows the new encryption scheme, otherwise use the previous encryption scheme */
      if ((dbPageSize >= 512) && (dbPageSize <= SQLITE_MAX_PAGE_SIZE) && (((dbPageSize - 1) & dbPageSize) == 0) &&
          (dbHeader[5] == 0x40) && (dbHeader[6] == 0x20) && (dbHeader[7] == 0x20))
      {
        /* Restore encrypted bytes 16..23 for new encryption scheme */
        memcpy(data + 16, data + 8, 8);
        offset = 16;
      }
    }
    rc = CodecAES128(aesCipher->m_aes, page, 0, aesCipher->m_key, data + offset, len - offset, data + offset);
    if (page == 1 && offset != 0)
    {
      /* Verify the database header */
      if (memcmp(dbHeader, data + 16, 8) == 0)
      {
        memcpy(data, SQLITE_FILE_HEADER, 16);
      }
    }
  }
  return rc;
}

/* --- AES 256-bit cipher (wxSQLite3) --- */

typedef struct _AES256Cipher
{
  int           m_legacy;
  int           m_legacyPageSize;
  int           m_kdfIter;
  int           m_keyLength;
  unsigned char m_key[KEYLENGTH_AES256];
  Rijndael*     m_aes;
} AES256Cipher;

void*
AllocateAES256Cipher(wx_sqlite3* db)
{
  AES256Cipher* aesCipher = (AES256Cipher*) wx_sqlite3_malloc(sizeof(AES256Cipher));
  if (aesCipher != NULL)
  {
    aesCipher->m_aes = (Rijndael*) wx_sqlite3_malloc(sizeof(Rijndael));
    if (aesCipher->m_aes != NULL)
    {
      aesCipher->m_keyLength = KEYLENGTH_AES256;
      memset(aesCipher->m_key, 0, KEYLENGTH_AES256);
      RijndaelCreate(aesCipher->m_aes);
    }
    else
    {
      wx_sqlite3_free(aesCipher);
      aesCipher = NULL;
    }
  }
  if (aesCipher != NULL)
  {
    CipherParams* cipherParams = (CipherParams*) GetCipherParams(db, CODEC_TYPE_AES256);
    aesCipher->m_legacy = GetCipherParameter(cipherParams, "legacy");
    aesCipher->m_legacyPageSize = GetCipherParameter(cipherParams, "legacy_page_size");
    aesCipher->m_kdfIter = GetCipherParameter(cipherParams, "kdf_iter");
  }
  return aesCipher;
}

void
FreeAES256Cipher(void* cipher)
{
  AES256Cipher* aesCipher = (AES256Cipher*) cipher;
  memset(aesCipher->m_aes, 0, sizeof(Rijndael));
  wx_sqlite3_free(aesCipher->m_aes);
  memset(aesCipher, 0, sizeof(AES256Cipher));
  wx_sqlite3_free(aesCipher);
}

void
CloneAES256Cipher(void* cipherTo, void* cipherFrom)
{
  AES256Cipher* aesCipherTo = (AES256Cipher*) cipherTo;
  AES256Cipher* aesCipherFrom = (AES256Cipher*) cipherFrom;
  aesCipherTo->m_legacy = aesCipherFrom->m_legacy;
  aesCipherTo->m_legacyPageSize = aesCipherFrom->m_legacyPageSize;
  aesCipherTo->m_kdfIter = aesCipherFrom->m_kdfIter;
  aesCipherTo->m_keyLength = aesCipherFrom->m_keyLength;
  memcpy(aesCipherTo->m_key, aesCipherFrom->m_key, KEYLENGTH_AES256);
  RijndaelInvalidate(aesCipherTo->m_aes);
  RijndaelInvalidate(aesCipherFrom->m_aes);
}

int
GetLegacyAES256Cipher(void* cipher)
{
  AES256Cipher* aesCipher = (AES256Cipher*)cipher;
  return aesCipher->m_legacy;
}

int
GetPageSizeAES256Cipher(void* cipher)
{
  AES256Cipher* aesCipher = (AES256Cipher*) cipher;
  int pageSize = 0;
  if (aesCipher->m_legacy != 0)
  {
    pageSize = aesCipher->m_legacyPageSize;
    if ((pageSize < 512) || (pageSize > SQLITE_MAX_PAGE_SIZE) || (((pageSize - 1) & pageSize) != 0))
    {
      pageSize = 0;
    }
  }
  return pageSize;
}

int
GetReservedAES256Cipher(void* cipher)
{
  return 0;
}

void
GenerateKeyAES256Cipher(void* cipher, Btree* pBt, char* userPassword, int passwordLength, int rekey)
{
  AES256Cipher* aesCipher = (AES256Cipher*) cipher;
  unsigned char userPad[32];
  unsigned char digest[KEYLENGTH_AES256];
  int keyLength = KEYLENGTH_AES256;
  int k;

  /* Pad password */
  CodecPadPassword(userPassword, passwordLength, userPad);

  sha256(userPad, 32, digest);
  for (k = 0; k < CODEC_SHA_ITER; ++k)
  {
    sha256(digest, KEYLENGTH_AES256, digest);
  }
  memcpy(aesCipher->m_key, digest, aesCipher->m_keyLength);
}

int
EncryptPageAES256Cipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  AES256Cipher* aesCipher = (AES256Cipher*) cipher;
  int rc = SQLITE_OK;
  if (aesCipher->m_legacy != 0)
  {
    /* Use the legacy encryption scheme */
    unsigned char* key = aesCipher->m_key;
    rc = CodecAES256(aesCipher->m_aes, page, 1, key, data, len, data);
  }
  else
  {
    unsigned char dbHeader[8];
    int offset = 0;
    unsigned char* key = aesCipher->m_key;
    if (page == 1)
    {
      /* Save the header bytes remaining unencrypted */
      memcpy(dbHeader, data + 16, 8);
      offset = 16;
      CodecAES256(aesCipher->m_aes, page, 1, key, data, 16, data);
    }
    rc = CodecAES256(aesCipher->m_aes, page, 1, key, data + offset, len - offset, data + offset);
    if (page == 1)
    {
      /* Move the encrypted header bytes 16..23 to a safe position */
      memcpy(data + 8, data + 16, 8);
      /* Restore the unencrypted header bytes 16..23 */
      memcpy(data + 16, dbHeader, 8);
    }
  }
  return rc;
}

int
DecryptPageAES256Cipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  AES256Cipher* aesCipher = (AES256Cipher*) cipher;
  int rc = SQLITE_OK;
  if (aesCipher->m_legacy != 0)
  {
    /* Use the legacy encryption scheme */
    rc = CodecAES256(aesCipher->m_aes, page, 0, aesCipher->m_key, data, len, data);
  }
  else
  {
    unsigned char dbHeader[8];
    int dbPageSize;
    int offset = 0;
    if (page == 1)
    {
      /* Save (unencrypted) header bytes 16..23 */
      memcpy(dbHeader, data + 16, 8);
      /* Determine page size */
      dbPageSize = (dbHeader[0] << 8) | (dbHeader[1] << 16);
      /* Check whether the database header is valid */
      /* If yes, the database follows the new encryption scheme, otherwise use the previous encryption scheme */
      if ((dbPageSize >= 512) && (dbPageSize <= SQLITE_MAX_PAGE_SIZE) && (((dbPageSize - 1) & dbPageSize) == 0) &&
        (dbHeader[5] == 0x40) && (dbHeader[6] == 0x20) && (dbHeader[7] == 0x20))
      {
        /* Restore encrypted bytes 16..23 for new encryption scheme */
        memcpy(data + 16, data + 8, 8);
        offset = 16;
      }
    }
    rc = CodecAES256(aesCipher->m_aes, page, 0, aesCipher->m_key, data + offset, len - offset, data + offset);
    if (page == 1 && offset != 0)
    {
      /* Verify the database header */
      if (memcmp(dbHeader, data + 16, 8) == 0)
      {
        memcpy(data, SQLITE_FILE_HEADER, 16);
      }
    }
  }
  return rc;
}

/* --- ChaCha20-Poly1305 cipher (plus sqleet variant) --- */

#define KEYLENGTH_CHACHA20       32
#define SALTLENGTH_CHACHA20      16
#define PAGE_NONCE_LEN_CHACHA20  16
#define PAGE_TAG_LEN_CHACHA20    16
#define PAGE_RESERVED_CHACHA20   (PAGE_NONCE_LEN_CHACHA20 + PAGE_TAG_LEN_CHACHA20)

typedef struct _chacha20Cipher
{
  int           m_legacy;
  int           m_legacyPageSize;
  int           m_kdfIter;
  int           m_keyLength;
  unsigned char m_key[KEYLENGTH_CHACHA20];
  unsigned char m_salt[SALTLENGTH_CHACHA20];
} ChaCha20Cipher;

void*
AllocateChaCha20Cipher(wx_sqlite3* db)
{
  ChaCha20Cipher* chacha20Cipher = (ChaCha20Cipher*) wx_sqlite3_malloc(sizeof(ChaCha20Cipher));
  if (chacha20Cipher != NULL)
  {
    chacha20Cipher->m_keyLength = KEYLENGTH_CHACHA20;
    memset(chacha20Cipher->m_key, 0, KEYLENGTH_CHACHA20);
    memset(chacha20Cipher->m_salt, 0, SALTLENGTH_CHACHA20);
  }
  if (chacha20Cipher != NULL)
  {
    CipherParams* cipherParams = (CipherParams*) GetCipherParams(db, CODEC_TYPE_CHACHA20);
    chacha20Cipher->m_legacy = GetCipherParameter(cipherParams, "legacy");
    chacha20Cipher->m_legacyPageSize = GetCipherParameter(cipherParams, "legacy_page_size");
    chacha20Cipher->m_kdfIter = GetCipherParameter(cipherParams, "kdf_iter");
    if (chacha20Cipher->m_legacy != 0)
    {
      chacha20Cipher->m_kdfIter = SQLEET_KDF_ITER;
    }
  }
  return chacha20Cipher;
}

void
FreeChaCha20Cipher(void* cipher)
{
  ChaCha20Cipher* chacha20Cipher = (ChaCha20Cipher*) cipher;
  memset(chacha20Cipher, 0, sizeof(ChaCha20Cipher));
  wx_sqlite3_free(chacha20Cipher);
}

void
CloneChaCha20Cipher(void* cipherTo, void* cipherFrom)
{
  ChaCha20Cipher* chacha20CipherTo = (ChaCha20Cipher*) cipherTo;
  ChaCha20Cipher* chacha20CipherFrom = (ChaCha20Cipher*) cipherFrom;
  chacha20CipherTo->m_legacy = chacha20CipherFrom->m_legacy;
  chacha20CipherTo->m_legacyPageSize = chacha20CipherFrom->m_legacyPageSize;
  chacha20CipherTo->m_kdfIter = chacha20CipherFrom->m_kdfIter;
  chacha20CipherTo->m_keyLength = chacha20CipherFrom->m_keyLength;
  memcpy(chacha20CipherTo->m_key, chacha20CipherFrom->m_key, KEYLENGTH_CHACHA20);
  memcpy(chacha20CipherTo->m_salt, chacha20CipherFrom->m_salt, SALTLENGTH_CHACHA20);
}

int
GetLegacyChaCha20Cipher(void* cipher)
{
  ChaCha20Cipher* chacha20Cipher = (ChaCha20Cipher*)cipher;
  return chacha20Cipher->m_legacy;
}

int
GetPageSizeChaCha20Cipher(void* cipher)
{
  ChaCha20Cipher* chacha20Cipher = (ChaCha20Cipher*) cipher;
  int pageSize = 0;
  if (chacha20Cipher->m_legacy != 0)
  {
    pageSize = chacha20Cipher->m_legacyPageSize;
    if ((pageSize < 512) || (pageSize > SQLITE_MAX_PAGE_SIZE) || (((pageSize - 1) & pageSize) != 0))
    {
      pageSize = 0;
    }
  }
  return pageSize;
}

int
GetReservedChaCha20Cipher(void* cipher)
{
  return PAGE_RESERVED_CHACHA20;
}

void
GenerateKeyChaCha20Cipher(void* cipher, Btree* pBt, char* userPassword, int passwordLength, int rekey)
{
  ChaCha20Cipher* chacha20Cipher = (ChaCha20Cipher*) cipher;

  Pager *pPager = pBt->pBt->pPager;
  wx_sqlite3_file* fd = (isOpen(pPager->fd)) ? pPager->fd : NULL;

  if (rekey || fd == NULL || wx_sqlite3OsRead(fd, chacha20Cipher->m_salt, SALTLENGTH_CHACHA20, 0) != SQLITE_OK)
  {
    chacha20_rng(chacha20Cipher->m_salt, SALTLENGTH_CHACHA20);
  }
  fastpbkdf2_hmac_sha256((unsigned char*) userPassword, passwordLength, 
                         chacha20Cipher->m_salt, SALTLENGTH_CHACHA20,
                         chacha20Cipher->m_kdfIter, 
                         chacha20Cipher->m_key, KEYLENGTH_CHACHA20);
}

int
EncryptPageChaCha20Cipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  ChaCha20Cipher* chacha20Cipher = (ChaCha20Cipher*) cipher;
  int rc = SQLITE_OK;
  int legacy = chacha20Cipher->m_legacy;
  int nReserved = (reserved == 0 && legacy == 0) ? 0 : GetReservedChaCha20Cipher(cipher);
  int n = len - nReserved;

  /* Generate one-time keys */
  unsigned char otk[64];
  uint32_t counter;
  int offset;

  /* Check whether number of required reserved bytes and actually reserved bytes match */
  if ((legacy == 0 && nReserved > reserved) || ((legacy != 0 && nReserved != reserved)))
  {
    return SQLITE_CORRUPT;
  }

  if (nReserved > 0)
  {
    /* Encrypt and authenticate */
    memset(otk, 0, 64);
    chacha20_rng(data + n, PAGE_NONCE_LEN_CHACHA20);
    counter = LOAD32_LE(data + n + PAGE_NONCE_LEN_CHACHA20 - 4) ^ page;
    chacha20_xor(otk, 64, chacha20Cipher->m_key, data + n, counter);

    offset = (page == 1) ? (chacha20Cipher->m_legacy != 0) ? 0 : CIPHER_PAGE1_OFFSET : 0;
    chacha20_xor(data + offset, n - offset, otk + 32, data + n, counter + 1);
    if (page == 1)
    {
      memcpy(data, chacha20Cipher->m_salt, SALTLENGTH_CHACHA20);
    }
    poly1305(data, n + PAGE_NONCE_LEN_CHACHA20, otk, data + n + PAGE_NONCE_LEN_CHACHA20);
  }
  else
  {
    /* Encrypt only */
    unsigned char nonce[PAGE_NONCE_LEN_CHACHA20];
    memset(otk, 0, 64);
    CodecGenerateInitialVector(page, nonce);
    counter = LOAD32_LE(&nonce[PAGE_NONCE_LEN_CHACHA20 - 4]) ^ page;
    chacha20_xor(otk, 64, chacha20Cipher->m_key, nonce, counter);

    /* Encrypt */
    offset = (page == 1) ? (chacha20Cipher->m_legacy != 0) ? 0 : CIPHER_PAGE1_OFFSET : 0;
    chacha20_xor(data + offset, n - offset, otk + 32, nonce, counter + 1);
    if (page == 1)
    {
      memcpy(data, chacha20Cipher->m_salt, SALTLENGTH_CHACHA20);
    }
  }

  return rc;
}

int
DecryptPageChaCha20Cipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  ChaCha20Cipher* chacha20Cipher = (ChaCha20Cipher*) cipher;
  int rc = SQLITE_OK;
  int legacy = chacha20Cipher->m_legacy;
  int nReserved = (reserved == 0 && legacy == 0) ? 0 : GetReservedChaCha20Cipher(cipher);
  int n = len - nReserved;

  /* Generate one-time keys */
  unsigned char otk[64];
  uint32_t counter;
  unsigned char tag[16];
  int offset;

  /* Check whether number of required reserved bytes and actually reserved bytes match */
  if ((legacy == 0 && nReserved > reserved) || ((legacy != 0 && nReserved != reserved)))
  {
    return SQLITE_CORRUPT;
  }

  if (nReserved > 0)
  {
    /* Decrypt and verify MAC */
    memset(otk, 0, 64);
    counter = LOAD32_LE(data + n + PAGE_NONCE_LEN_CHACHA20 - 4) ^ page;
    chacha20_xor(otk, 64, chacha20Cipher->m_key, data + n, counter);

    /* Verify the MAC */
    poly1305(data, n + PAGE_NONCE_LEN_CHACHA20, otk, tag);
    if (!poly1305_tagcmp(data + n + PAGE_NONCE_LEN_CHACHA20, tag))
    {
      /* Decrypt */
      offset = (page == 1) ? (chacha20Cipher->m_legacy != 0) ? 0 : CIPHER_PAGE1_OFFSET : 0;
      chacha20_xor(data + offset, n - offset, otk + 32, data + n, counter + 1);
      if (page == 1)
      {
        memcpy(data, SQLITE_FILE_HEADER, 16);
      }
    }
    else
    {
      /* Bad MAC */
      rc = SQLITE_CORRUPT;
    }
  }
  else
  {
    /* Decrypt only */
    unsigned char nonce[PAGE_NONCE_LEN_CHACHA20];
    memset(otk, 0, 64);
    CodecGenerateInitialVector(page, nonce);
    counter = LOAD32_LE(&nonce[PAGE_NONCE_LEN_CHACHA20 - 4]) ^ page;
    chacha20_xor(otk, 64, chacha20Cipher->m_key, nonce, counter);

    /* Decrypt */
    offset = (page == 1) ? (chacha20Cipher->m_legacy != 0) ? 0 : CIPHER_PAGE1_OFFSET : 0;
    chacha20_xor(data + offset, n - offset, otk + 32, nonce, counter + 1);
    if (page == 1)
    {
      memcpy(data, SQLITE_FILE_HEADER, 16);
    }
  }

  return rc;
}

/* --- SQLCipher AES256CBC-HMAC cipher --- */

#define KEYLENGTH_SQLCIPHER      32
#define SALTLENGTH_SQLCIPHER     16
#define HMAC_LENGTH_SQLCIPHER    SHA1_DIGEST_SIZE
#define PAGE_NONCE_LEN_SQLCIPHER 16

typedef struct _sqlCipherCipher
{
  int           m_legacy;
  int           m_legacyPageSize;
  int           m_kdfIter;
  int           m_fastKdfIter;
  int           m_hmacUse;
  int           m_hmacPgno;
  int           m_hmacSaltMask;
  int           m_keyLength;
  unsigned char m_key[KEYLENGTH_SQLCIPHER];
  unsigned char m_salt[SALTLENGTH_SQLCIPHER];
  unsigned char m_hmacKey[KEYLENGTH_SQLCIPHER];
  Rijndael*     m_aes;
} SQLCipherCipher;

void*
AllocateSQLCipherCipher(wx_sqlite3* db)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*) wx_sqlite3_malloc(sizeof(SQLCipherCipher));
  if (sqlCipherCipher != NULL)
  {
    sqlCipherCipher->m_aes = (Rijndael*)wx_sqlite3_malloc(sizeof(Rijndael));
    if (sqlCipherCipher->m_aes != NULL)
    {
      sqlCipherCipher->m_keyLength = KEYLENGTH_SQLCIPHER;
      memset(sqlCipherCipher->m_key, 0, KEYLENGTH_SQLCIPHER);
      memset(sqlCipherCipher->m_salt, 0, SALTLENGTH_SQLCIPHER);
      memset(sqlCipherCipher->m_hmacKey, 0, KEYLENGTH_AES256);
      RijndaelCreate(sqlCipherCipher->m_aes);
    }
    else
    {
      wx_sqlite3_free(sqlCipherCipher);
      sqlCipherCipher = NULL;
    }
  }
  if (sqlCipherCipher != NULL)
  {
    CipherParams* cipherParams = (CipherParams*) GetCipherParams(db, CODEC_TYPE_SQLCIPHER);
    sqlCipherCipher->m_legacy = GetCipherParameter(cipherParams, "legacy");
    sqlCipherCipher->m_legacyPageSize = GetCipherParameter(cipherParams, "legacy_page_size");
    sqlCipherCipher->m_kdfIter = GetCipherParameter(cipherParams, "kdf_iter");
    sqlCipherCipher->m_fastKdfIter = GetCipherParameter(cipherParams, "fast_kdf_iter");
    sqlCipherCipher->m_hmacUse = GetCipherParameter(cipherParams, "hmac_use");
    sqlCipherCipher->m_hmacPgno = GetCipherParameter(cipherParams, "hmac_pgno");
    sqlCipherCipher->m_hmacSaltMask = GetCipherParameter(cipherParams, "hmac_salt_mask");
  }
  return sqlCipherCipher;
}

void
FreeSQLCipherCipher(void* cipher)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*) cipher;
  memset(sqlCipherCipher->m_aes, 0, sizeof(Rijndael));
  wx_sqlite3_free(sqlCipherCipher->m_aes);
  memset(sqlCipherCipher, 0, sizeof(SQLCipherCipher));
  wx_sqlite3_free(sqlCipherCipher);
}

void
CloneSQLCipherCipher(void* cipherTo, void* cipherFrom)
{
  SQLCipherCipher* sqlCipherCipherTo = (SQLCipherCipher*) cipherTo;
  SQLCipherCipher* sqlCipherCipherFrom = (SQLCipherCipher*) cipherFrom;
  sqlCipherCipherTo->m_legacy = sqlCipherCipherFrom->m_legacy;
  sqlCipherCipherTo->m_legacyPageSize = sqlCipherCipherFrom->m_legacyPageSize;
  sqlCipherCipherTo->m_kdfIter = sqlCipherCipherFrom->m_kdfIter;
  sqlCipherCipherTo->m_fastKdfIter = sqlCipherCipherFrom->m_fastKdfIter;
  sqlCipherCipherTo->m_hmacUse = sqlCipherCipherFrom->m_hmacUse;
  sqlCipherCipherTo->m_hmacPgno = sqlCipherCipherFrom->m_hmacPgno;
  sqlCipherCipherTo->m_hmacSaltMask = sqlCipherCipherFrom->m_hmacSaltMask;
  sqlCipherCipherTo->m_keyLength = sqlCipherCipherFrom->m_keyLength;
  memcpy(sqlCipherCipherTo->m_key, sqlCipherCipherFrom->m_key, KEYLENGTH_SQLCIPHER);
  memcpy(sqlCipherCipherTo->m_salt, sqlCipherCipherFrom->m_salt, SALTLENGTH_SQLCIPHER);
  memcpy(sqlCipherCipherTo->m_hmacKey, sqlCipherCipherFrom->m_hmacKey, KEYLENGTH_SQLCIPHER);
  RijndaelInvalidate(sqlCipherCipherTo->m_aes);
  RijndaelInvalidate(sqlCipherCipherFrom->m_aes);
}

int
GetLegacySQLCipherCipher(void* cipher)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*)cipher;
  return sqlCipherCipher->m_legacy;
}

int
GetPageSizeSQLCipherCipher(void* cipher)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*) cipher;
  int pageSize = 0;
  if (sqlCipherCipher->m_legacy != 0)
  {
    pageSize = sqlCipherCipher->m_legacyPageSize;
    if ((pageSize < 512) || (pageSize > SQLITE_MAX_PAGE_SIZE) || (((pageSize - 1) & pageSize) != 0))
    {
      pageSize = 0;
    }
  }
  return pageSize;
}

int
GetReservedSQLCipherCipher(void* cipher)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*) cipher;
  int reserved = SALTLENGTH_SQLCIPHER;
  if (sqlCipherCipher->m_hmacUse != 0)
  {
    reserved += 32;
  }
  return reserved;
}

static int IsHexKey(const unsigned char* hex, int len)
{
  int j;
  for (j = 0; j < len; ++j)
  {
    unsigned char c = hex[j];
    if ((c < '0' || c > '9') && (c < 'A' || c > 'F') && (c < 'a' || c > 'f'))
    {
      return 0;
    }
  }
  return 1;
}

static int ConvertHex2Int(char c)
{
  return (c >= '0' && c <= '9') ? (c)-'0' :
         (c >= 'A' && c <= 'F') ? (c)-'A' + 10 :
         (c >= 'a' && c <= 'f') ? (c)-'a' + 10 : 0;
}

static void ConvertHex2Bin(const unsigned char* hex, int len, unsigned char* bin)
{
  int j;
  for (j = 0; j < len; j += 2)
  {
    bin[j / 2] = (ConvertHex2Int(hex[j]) << 4) | ConvertHex2Int(hex[j + 1]);
  }
}

void
GenerateKeySQLCipherCipher(void* cipher, Btree* pBt, char* userPassword, int passwordLength, int rekey)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*) cipher;

  Pager *pPager = pBt->pBt->pPager;
  wx_sqlite3_file* fd = (isOpen(pPager->fd)) ? pPager->fd : NULL;

  if (rekey || fd == NULL || wx_sqlite3OsRead(fd, sqlCipherCipher->m_salt, SALTLENGTH_SQLCIPHER, 0) != SQLITE_OK)
  {
    chacha20_rng(sqlCipherCipher->m_salt, SALTLENGTH_SQLCIPHER);
  }

  if (passwordLength == ((KEYLENGTH_SQLCIPHER * 2) + 3) &&
    wx_sqlite3_strnicmp(userPassword, "x'", 2) == 0 &&
    IsHexKey((unsigned char*) (userPassword + 2), KEYLENGTH_SQLCIPHER * 2) != 0)
  {
    ConvertHex2Bin((unsigned char*) (userPassword + 2), passwordLength - 3, sqlCipherCipher->m_key);
  }
  else if (passwordLength == (((KEYLENGTH_SQLCIPHER + SALTLENGTH_SQLCIPHER) * 2) + 3) &&
    wx_sqlite3_strnicmp(userPassword, "x'", 2) == 0 &&
    IsHexKey((unsigned char*) (userPassword + 2), (KEYLENGTH_SQLCIPHER + SALTLENGTH_SQLCIPHER) * 2) != 0)
  {
    ConvertHex2Bin((unsigned char*) (userPassword + 2), KEYLENGTH_SQLCIPHER * 2, sqlCipherCipher->m_key);
    ConvertHex2Bin((unsigned char*) (userPassword + 2 + KEYLENGTH_SQLCIPHER * 2), SALTLENGTH_SQLCIPHER * 2, sqlCipherCipher->m_salt);
  }
  else
  {
    fastpbkdf2_hmac_sha1((unsigned char*) userPassword, passwordLength,
                         sqlCipherCipher->m_salt, SALTLENGTH_SQLCIPHER,
                         sqlCipherCipher->m_kdfIter,
                         sqlCipherCipher->m_key, KEYLENGTH_SQLCIPHER);
  }

  if (sqlCipherCipher->m_hmacUse != 0)
  {
    int j;
    unsigned char hmacSaltMask = sqlCipherCipher->m_hmacSaltMask;
    unsigned char hmacSalt[SALTLENGTH_SQLCIPHER];
    memcpy(hmacSalt, sqlCipherCipher->m_salt, SALTLENGTH_SQLCIPHER);
    for (j = 0; j < SALTLENGTH_SQLCIPHER; ++j)
    {
      hmacSalt[j] ^= hmacSaltMask;
    }
    fastpbkdf2_hmac_sha1(sqlCipherCipher->m_key, KEYLENGTH_SQLCIPHER,
                         hmacSalt, SALTLENGTH_SQLCIPHER,
                         sqlCipherCipher->m_fastKdfIter,
                         sqlCipherCipher->m_hmacKey, KEYLENGTH_SQLCIPHER);
  }
}

int
EncryptPageSQLCipherCipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*) cipher;
  int rc = SQLITE_OK;
  int legacy = sqlCipherCipher->m_legacy;
  int nReserved = (reserved == 0 && legacy == 0) ? 0 : GetReservedSQLCipherCipher(cipher);
  int n = len - nReserved;
  int offset = (page == 1) ? (sqlCipherCipher->m_legacy != 0) ? 16 : 24 : 0;
  int blen;
  unsigned char iv[64];

  /* Check whether number of required reserved bytes and actually reserved bytes match */
  if ((legacy == 0 && nReserved > reserved) || ((legacy != 0 && nReserved != reserved)))
  {
    return SQLITE_CORRUPT;
  }

  /* Generate nonce (64 bytes) */
  memset(iv, 0, 64);
  if (nReserved > 0)
  {
    chacha20_rng(iv, 64);
  }
  else
  {
    CodecGenerateInitialVector(page, iv);
  }

  RijndaelInit(sqlCipherCipher->m_aes, RIJNDAEL_Direction_Mode_CBC, RIJNDAEL_Direction_Encrypt, sqlCipherCipher->m_key, RIJNDAEL_Direction_KeyLength_Key32Bytes, iv);
  blen = RijndaelBlockEncrypt(sqlCipherCipher->m_aes, data + offset, (n - offset) * 8, data + offset);
  if (nReserved > 0)
  {
    memcpy(data + n, iv, nReserved);
  }
  if (page == 1)
  {
    memcpy(data, sqlCipherCipher->m_salt, SALTLENGTH_SQLCIPHER);
  }

  /* hmac calculation */
  if (sqlCipherCipher->m_hmacUse == 1 && nReserved > 0)
  {
    unsigned char pgno_raw[4];
    unsigned char hmac_out[64];
    if (sqlCipherCipher->m_hmacPgno == SQLCIPHER_HMAC_PGNO_LE)
    {
      STORE32_LE(pgno_raw, page);
    }
    else if (sqlCipherCipher->m_hmacPgno == SQLCIPHER_HMAC_PGNO_BE)
    {
      STORE32_BE(pgno_raw, page);
    }
    else
    {
      memcpy(pgno_raw, &page, 4);
    }
    sqlcipher_hmac(sqlCipherCipher->m_hmacKey, KEYLENGTH_SQLCIPHER, data + offset, n + PAGE_NONCE_LEN_SQLCIPHER - offset, pgno_raw, 4, hmac_out);
    memcpy(data + n + PAGE_NONCE_LEN_SQLCIPHER, hmac_out, HMAC_LENGTH_SQLCIPHER);
  }

  return rc;
}

int
DecryptPageSQLCipherCipher(void* cipher, int page, unsigned char* data, int len, int reserved)
{
  SQLCipherCipher* sqlCipherCipher = (SQLCipherCipher*) cipher;
  int rc = SQLITE_OK;
  int legacy = sqlCipherCipher->m_legacy;
  int nReserved = (reserved == 0 && legacy == 0) ? 0 : GetReservedSQLCipherCipher(cipher);
  int n = len - nReserved;
  int offset = (page == 1) ? (sqlCipherCipher->m_legacy != 0) ? 16 : 24 : 0;
  int hmacOk = 1;
  int blen;
  unsigned char iv[64];

  /* Check whether number of required reserved bytes and actually reserved bytes match */
  if ((legacy == 0 && nReserved > reserved) || ((legacy != 0 && nReserved != reserved)))
  {
    return SQLITE_CORRUPT;
  }

  /* Get nonce from buffer */
  if (nReserved > 0)
  {
    memcpy(iv, data + n, nReserved);
  }
  else
  {
    CodecGenerateInitialVector(page, iv);
  }

  /* hmac check */
  if (sqlCipherCipher->m_hmacUse == 1 && nReserved > 0)
  {
    unsigned char pgno_raw[4];
    unsigned char hmac_out[64];
    if (sqlCipherCipher->m_hmacPgno == SQLCIPHER_HMAC_PGNO_LE)
    {
      STORE32_LE(pgno_raw, page);
    }
    else if (sqlCipherCipher->m_hmacPgno == SQLCIPHER_HMAC_PGNO_BE)
    {
      STORE32_BE(pgno_raw, page);
    }
    else
    {
      memcpy(pgno_raw, &page, 4);
    }
    sqlcipher_hmac(sqlCipherCipher->m_hmacKey, KEYLENGTH_SQLCIPHER, data + offset, n + PAGE_NONCE_LEN_SQLCIPHER - offset, pgno_raw, 4, hmac_out);
    hmacOk = (memcmp(data + n + PAGE_NONCE_LEN_SQLCIPHER, hmac_out, HMAC_LENGTH_SQLCIPHER) == 0);
  }

  if (hmacOk != 0)
  {
    RijndaelInit(sqlCipherCipher->m_aes, RIJNDAEL_Direction_Mode_CBC, RIJNDAEL_Direction_Decrypt, sqlCipherCipher->m_key, RIJNDAEL_Direction_KeyLength_Key32Bytes, iv);
    blen = RijndaelBlockDecrypt(sqlCipherCipher->m_aes, data + offset, (n - offset) * 8, data + offset);
    if (nReserved > 0)
    {
      memcpy(data + n, iv, nReserved);
    }
    if (page == 1)
    {
      memcpy(data, SQLITE_FILE_HEADER, 16);
    }
  }
  else
  {
    /* Bad MAC */
    rc = SQLITE_CORRUPT;
  }

  return rc;
}


typedef struct _CodecParameter
{
  char*         m_name;
  CipherParams* m_params;
} CodecParameter;

CodecParameter codecParameterTable[] =
{
  { "global",    commonParams },
  { "aes128cbc", aes128Params },
  { "aes256cbc", aes256Params },
  { "chacha20",  chacha20Params },
  { "sqlcipher", sqlCipherParams },
  { "",          NULL }
};

CodecParameter*
CloneCodecParameterTable()
{
  /* Count number of codecs and cipher parameters */
  int nTables = 0;
  int nParams = 0;
  int j, k, n;
  CipherParams* cloneCipherParams;
  CodecParameter* cloneCodecParams;

  for (j = 0; strlen(codecParameterTable[j].m_name) > 0; ++j)
  {
    CipherParams* params = codecParameterTable[j].m_params;
    for (k = 0; strlen(params[k].m_name) > 0; ++k);
    nParams += k;
  }
  nTables = j;

  /* Allocate memory for cloned codec parameter tables (including sentinel for each table) */
  cloneCipherParams = (CipherParams*) wx_sqlite3_malloc((nParams + nTables) * sizeof(CipherParams));
  cloneCodecParams = (CodecParameter*) wx_sqlite3_malloc((nTables + 1) * sizeof(CodecParameter));

  /* Create copy of tables */
  if (cloneCodecParams != NULL)
  {
    int offset = 0;
    for (j = 0; j < nTables; ++j)
    {
      CipherParams* params = codecParameterTable[j].m_params;
      cloneCodecParams[j].m_name = codecParameterTable[j].m_name;
      cloneCodecParams[j].m_params = &cloneCipherParams[offset];
      for (n = 0; strlen(params[n].m_name) > 0; ++n);
      /* Copy all parameters of the current table (including sentinel) */
      for (k = 0; k <= n; ++k)
      {
        cloneCipherParams[offset + k].m_name     = params[k].m_name;
        cloneCipherParams[offset + k].m_value    = params[k].m_value;
        cloneCipherParams[offset + k].m_default  = params[k].m_default;
        cloneCipherParams[offset + k].m_minValue = params[k].m_minValue;
        cloneCipherParams[offset + k].m_maxValue = params[k].m_maxValue;
      }
      offset += (n + 1);
    }
    cloneCodecParams[nTables].m_name = codecParameterTable[nTables].m_name;
    cloneCodecParams[nTables].m_params = NULL;
  }
  else
  {
    wx_sqlite3_free(cloneCipherParams);
  }
  return cloneCodecParams;
}

void
FreeCodecParameterTable(CodecParameter* codecParams)
{
  wx_sqlite3_free(codecParams[0].m_params);
  wx_sqlite3_free(codecParams);
}

typedef void* (*AllocateCipher_t)(wx_sqlite3* db);
typedef void  (*FreeCipher_t)(void* cipher);
typedef void  (*CloneCipher_t)(void* cipherTo, void* cipherFrom);
typedef int   (*GetLegacy_t)(void* cipher);
typedef int   (*GetPageSize_t)(void* cipher);
typedef int   (*GetReserved_t)(void* cipher);
typedef void  (*GenerateKey_t)(void* cipher, Btree* pBt, char* userPassword, int passwordLength, int rekey);
typedef int   (*EncryptPage_t)(void* cipher, int page, unsigned char* data, int len, int reserved);
typedef int   (*DecryptPage_t)(void* cipher, int page, unsigned char* data, int len, int reserved);

typedef struct _CodecDescriptor
{
  char             m_name[32];
  AllocateCipher_t m_allocateCipher;
  FreeCipher_t     m_freeCipher;
  CloneCipher_t    m_cloneCipher;
  GetLegacy_t      m_getLegacy;
  GetPageSize_t    m_getPageSize;
  GetReserved_t    m_getReserved;
  GenerateKey_t    m_generateKey;
  EncryptPage_t    m_encryptPage;
  DecryptPage_t    m_decryptPage;
} CodecDescriptor;

CodecDescriptor codecDescriptorTable[] =
{
  /* wxSQLite3 AES 128 bit CBC */
  { "aes128cbc", AllocateAES128Cipher,
                 FreeAES128Cipher,
                 CloneAES128Cipher,
                 GetLegacyAES128Cipher,
                 GetPageSizeAES128Cipher,
                 GetReservedAES128Cipher,
                 GenerateKeyAES128Cipher,
                 EncryptPageAES128Cipher,
                 DecryptPageAES128Cipher },
  /* wxSQLite3 AES 128 bit CBC */
  { "aes256cbc", AllocateAES256Cipher,
                 FreeAES256Cipher,
                 CloneAES256Cipher,
                 GetLegacyAES256Cipher,
                 GetPageSizeAES256Cipher,
                 GetReservedAES256Cipher,
                 GenerateKeyAES256Cipher,
                 EncryptPageAES256Cipher,
                 DecryptPageAES256Cipher },
  /* ChaCha20 - Poly1305 (including sqleet legacy */
  { "chacha20",  AllocateChaCha20Cipher,
                 FreeChaCha20Cipher,
                 CloneChaCha20Cipher,
                 GetLegacyChaCha20Cipher,
                 GetPageSizeChaCha20Cipher,
                 GetReservedChaCha20Cipher,
                 GenerateKeyChaCha20Cipher,
                 EncryptPageChaCha20Cipher,
                 DecryptPageChaCha20Cipher },
  /* ChaCha20 - Poly1305 (including sqleet legacy */
  { "sqlcipher", AllocateSQLCipherCipher,
                 FreeSQLCipherCipher,
                 CloneSQLCipherCipher,
                 GetLegacySQLCipherCipher,
                 GetPageSizeSQLCipherCipher,
                 GetReservedSQLCipherCipher,
                 GenerateKeySQLCipherCipher,
                 EncryptPageSQLCipherCipher,
                 DecryptPageSQLCipherCipher },
  { "", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

/* --- Codec --- */

void
wxwx_sqlite3_config_table(wx_sqlite3_context* context, int argc, wx_sqlite3_value** argv)
{
  CodecParameter* codecParams = (CodecParameter*) wx_sqlite3_user_data(context);
  assert(argc == 0);
  wx_sqlite3_result_pointer(context, codecParams, "wxwx_sqlite3_codec_params", 0);
}

/*
** smallest integer value not less than argument
*/
void
wxwx_sqlite3_config_params(wx_sqlite3_context* context, int argc, wx_sqlite3_value** argv)
{
  CodecParameter* codecParams;
  const char* nameParam1;
  int hasDefaultPrefix = 0;
  int hasMinPrefix = 0;
  int hasMaxPrefix = 0;
  CipherParams* param1;
  CipherParams* cipherParamTable = NULL;
  int isCommonParam1;
  int isCipherParam1 = 0;

  assert(argc == 1 || argc == 2 || argc == 3);
  /* NULL values are not allowed for the first 2 arguments */
  if (SQLITE_NULL == wx_sqlite3_value_type(argv[0]) || (argc > 1 && SQLITE_NULL == wx_sqlite3_value_type(argv[1])))
  {
    wx_sqlite3_result_null(context);
    return;
  }

  codecParams = (CodecParameter*) wx_sqlite3_user_data(context);

  /* Check first argument whether it is a common parameter */
  /* If the first argument is a common parameter, param1 will point to its parameter table entry */
  nameParam1 = (const char*) wx_sqlite3_value_text(argv[0]);
  if (wx_sqlite3_strnicmp(nameParam1, "default:", 8) == 0)
  {
    hasDefaultPrefix = 1;
    nameParam1 += 8;
  }
  if (wx_sqlite3_strnicmp(nameParam1, "min:", 4) == 0)
  {
    hasMinPrefix = 1;
    nameParam1 += 4;
  }
  if (wx_sqlite3_strnicmp(nameParam1, "max:", 4) == 0)
  {
    hasMaxPrefix = 1;
    nameParam1 += 4;
  }

  param1 = codecParams[0].m_params;
  cipherParamTable = NULL;
  for (; strlen(param1->m_name) > 0; ++param1)
  {
    if (wx_sqlite3_stricmp(nameParam1, param1->m_name) == 0) break;
  }
  isCommonParam1 = strlen(param1->m_name) > 0;

  /* Check first argument whether it is a cipher name, if it wasn't a common parameter */
  /* If the first argument is a cipher name, cipherParamTable will point to the corresponding cipher parameter table */
  if (!isCommonParam1)
  {
    if (!hasDefaultPrefix && !hasMinPrefix && !hasMaxPrefix)
    {
      int j = 0;
      for (j = 0; strlen(codecParams[j].m_name) > 0; ++j)
      {
        if (wx_sqlite3_stricmp(nameParam1, codecParams[j].m_name) == 0) break;
      }
      isCipherParam1 = strlen(codecParams[j].m_name) > 0;
      if (isCipherParam1)
      {
        cipherParamTable = codecParams[j].m_params;
      }
    }
    if (!isCipherParam1)
    {
      /* Prefix not allowed for cipher names or cipher name not found */
      wx_sqlite3_result_null(context);
      return;
    }
  }

  if (argc == 1)
  {
    /* Return value of param1 */
    if (isCommonParam1)
    {
      int value = (hasDefaultPrefix) ? param1->m_default : (hasMinPrefix) ? param1->m_minValue : (hasMaxPrefix) ? param1->m_maxValue : param1->m_value;
      if (wx_sqlite3_stricmp(nameParam1, "cipher") == 0)
      {
        wx_sqlite3_result_text(context, codecDescriptorTable[value-1].m_name, -1, SQLITE_STATIC);
      }
      else
      {
        wx_sqlite3_result_int(context, value);
      }
    }
    else if (isCipherParam1)
    {
      /* Return a list of available parameters for the requested cipher */
      int nParams = 0;
      int lenTotal = 0;
      int j;
      for (j = 0; strlen(cipherParamTable[j].m_name) > 0; ++j)
      {
        ++nParams;
        lenTotal += strlen(cipherParamTable[j].m_name);
      }
      if (nParams > 0)
      {
        char* paramList = (char*) wx_sqlite3_malloc(lenTotal + nParams);
        if (paramList != NULL)
        {
          char* p = paramList;
          strcpy(paramList, cipherParamTable[0].m_name);
          for (j = 1; j < nParams; ++j)
          {
            strcat(paramList, ",");
            strcat(paramList, cipherParamTable[j].m_name);
          }
          wx_sqlite3_result_text(context, paramList, -1, wx_sqlite3_free);
        }
        else
        {
          /* Not enough memory to allocate the result */
          wx_sqlite3_result_error_nomem(context);
        }
      }
      else
      {
        /* Cipher has no parameters */
        wx_sqlite3_result_null(context);
      }
    }
  }
  else
  {
    int arg2Type = wx_sqlite3_value_type(argv[1]);
    if (argc == 2 && isCommonParam1)
    {
      /* Set value of common parameter */
      if (wx_sqlite3_stricmp(nameParam1, "cipher") == 0)
      {
        /* 2nd argument is a cipher name */
        if (arg2Type == SQLITE_TEXT)
        {
          const char* nameCipher = (const char*) wx_sqlite3_value_text(argv[1]);
          int j = 0;
          for (j = 0; strlen(codecDescriptorTable[j].m_name) > 0; ++j)
          {
            if (wx_sqlite3_stricmp(nameCipher, codecDescriptorTable[j].m_name) == 0) break;
          }
          if (strlen(codecDescriptorTable[j].m_name) > 0)
          {
            if (hasDefaultPrefix)
            {
              param1->m_default = j + 1;
            }
            param1->m_value = j + 1;
            wx_sqlite3_result_text(context, codecDescriptorTable[j].m_name, -1, SQLITE_STATIC);
          }
          else
          {
            /* No match for cipher name found */
            wx_sqlite3_result_null(context);
          }
        }
        else
        {
          /* Invalid parameter type */
          wx_sqlite3_result_null(context);
        }
      }
      else if (arg2Type == SQLITE_INTEGER)
      {
        /* Check that parameter value is within allowed range */
        int value = wx_sqlite3_value_int(argv[1]);
        if (value >= param1->m_minValue && value <= param1->m_maxValue)
        {
          if (hasDefaultPrefix)
          {
            param1->m_default = value;
          }
          param1->m_value = value;
          wx_sqlite3_result_int(context, value);
        }
        else
        {
          /* Parameter value not within allowed range */
          wx_sqlite3_result_null(context);
        }
      }
      else
      {
        wx_sqlite3_result_null(context);
      }
    }
    else if (isCipherParam1 && arg2Type == SQLITE_TEXT)
    {
      const char* nameParam2 = (const char*) wx_sqlite3_value_text(argv[1]);
      CipherParams* param2 = cipherParamTable;
      hasDefaultPrefix = 0;
      if (wx_sqlite3_strnicmp(nameParam2, "default:", 8) == 0)
      {
        hasDefaultPrefix = 1;
        nameParam2 += 8;
      }
      hasMinPrefix = 0;
      if (wx_sqlite3_strnicmp(nameParam2, "min:", 4) == 0)
      {
        hasMinPrefix = 1;
        nameParam2 += 4;
      }
      hasMaxPrefix = 0;
      if (wx_sqlite3_strnicmp(nameParam2, "max:", 4) == 0)
      {
        hasMaxPrefix = 1;
        nameParam2 += 4;
      }
      for (; strlen(param2->m_name) > 0; ++param2)
      {
        if (wx_sqlite3_stricmp(nameParam2, param2->m_name) == 0) break;
      }
      if (strlen(param2->m_name) > 0)
      {
        if (argc == 2)
        {
          /* Return parameter value */
          int value = (hasDefaultPrefix) ? param2->m_default : (hasMinPrefix) ? param2->m_minValue : (hasMaxPrefix) ? param2->m_maxValue : param2->m_value;
          wx_sqlite3_result_int(context, value);
        }
        else if (!hasMinPrefix && !hasMaxPrefix && wx_sqlite3_value_type(argv[2]) == SQLITE_INTEGER)
        {
          /* Change cipher parameter value */
          int value = wx_sqlite3_value_int(argv[2]);
          if (value >= param2->m_minValue && value <= param2->m_maxValue)
          {
            if (hasDefaultPrefix)
            {
              param2->m_default = value;
            }
            param2->m_value = value;
            wx_sqlite3_result_int(context, value);
          }
          else
          {
            /* Cipher parameter value not within allowed range */
            wx_sqlite3_result_null(context);
          }
        }
        else
        {
          /* Only current value and default value of a parameter can be changed */
          wx_sqlite3_result_null(context);
        }
      }
      else
      {
        /* Cipher parameter not found */
        wx_sqlite3_result_null(context);
      }
    }
    else
    {
      /* Cipher has no parameters */
      wx_sqlite3_result_null(context);
    }
  }
}

CodecParameter*
GetCodecParams(wx_sqlite3* db)
{
#if 0
  wx_sqlite3_mutex_enter(db->mutex);
#endif
  CodecParameter* codecParams = NULL;
  wx_sqlite3_stmt* pStmt = 0; 
  int rc = wx_sqlite3_prepare_v2(db, "SELECT wxwx_sqlite3_config_table();", -1, &pStmt, 0); 
  if (rc == SQLITE_OK)
  {
    if (SQLITE_ROW == wx_sqlite3_step(pStmt))
    {
      wx_sqlite3_value* ptrValue = wx_sqlite3_column_value(pStmt, 0);
      codecParams = (CodecParameter*) wx_sqlite3_value_pointer(ptrValue, "wxwx_sqlite3_codec_params");
    }
    wx_sqlite3_finalize(pStmt); 
  }
#if 0
  wx_sqlite3_mutex_leave(db->mutex);
#endif
  return codecParams;
}

int
wxwx_sqlite3_config(wx_sqlite3* db, const char* paramName, int newValue)
{
  int value = -1;
  CodecParameter* codecParams;
  int hasDefaultPrefix = 0;
  int hasMinPrefix = 0;
  int hasMaxPrefix = 0;
  CipherParams* param;

  if (paramName == NULL || (db == NULL && newValue >= 0))
  {
    return value;
  }

  codecParams = (db != NULL) ? GetCodecParams(db) : codecParameterTable;
  if (codecParams == NULL)
  {
    return value;
  }

  if (wx_sqlite3_strnicmp(paramName, "default:", 8) == 0)
  {
    hasDefaultPrefix = 1;
    paramName += 8;
  }
  if (wx_sqlite3_strnicmp(paramName, "min:", 4) == 0)
  {
    hasMinPrefix = 1;
    paramName += 4;
  }
  if (wx_sqlite3_strnicmp(paramName, "max:", 4) == 0)
  {
    hasMaxPrefix = 1;
    paramName += 4;
  }

  param = codecParams[0].m_params;
  for (; strlen(param->m_name) > 0; ++param)
  {
    if (wx_sqlite3_stricmp(paramName, param->m_name) == 0) break;
  }
  if (strlen(param->m_name) > 0)
  {
    if (db != NULL)
    {
      wx_sqlite3_mutex_enter(db->mutex);
    }
    else
    {
      wx_sqlite3_mutex_enter(wx_sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER));
    }
    value = (hasDefaultPrefix) ? param->m_default : (hasMinPrefix) ? param->m_minValue : (hasMaxPrefix) ? param->m_maxValue : param->m_value;
    if (!hasMinPrefix && !hasMaxPrefix && newValue >= 0 && newValue >= param->m_minValue && newValue <= param->m_maxValue)
    {
      if (hasDefaultPrefix)
      {
        param->m_default = newValue;
      }
      param->m_value = newValue;
      value = newValue;
    }
    if (db != NULL)
    {
      wx_sqlite3_mutex_leave(db->mutex);
    }
    else
    {
      wx_sqlite3_mutex_leave(wx_sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER));
    }
  }
  return value;
}

int
wxwx_sqlite3_config_cipher(wx_sqlite3* db, const char* cipherName, const char* paramName, int newValue)
{
  int value = -1;
  CodecParameter* codecParams;
  CipherParams* cipherParamTable = NULL;
  int j = 0;

  if (cipherName == NULL || paramName == NULL || (db == NULL && newValue >= 0))
  {
    return value;
  }

  codecParams = (db != NULL) ? GetCodecParams(db) : codecParameterTable;
  if (codecParams == NULL)
  {
    return value;
  }

  for (j = 0; strlen(codecParams[j].m_name) > 0; ++j)
  {
    if (wx_sqlite3_stricmp(cipherName, codecParams[j].m_name) == 0) break;
  }
  if (strlen(codecParams[j].m_name) > 0)
  {
    cipherParamTable = codecParams[j].m_params;
  }

  if (cipherParamTable != NULL)
  {
    int hasDefaultPrefix = 0;
    int hasMinPrefix = 0;
    int hasMaxPrefix = 0;
    CipherParams* param = cipherParamTable;

    if (wx_sqlite3_strnicmp(paramName, "default:", 8) == 0)
    {
      hasDefaultPrefix = 1;
      paramName += 8;
    }
    if (wx_sqlite3_strnicmp(paramName, "min:", 4) == 0)
    {
      hasMinPrefix = 1;
      paramName += 4;
    }
    if (wx_sqlite3_strnicmp(paramName, "max:", 4) == 0)
    {
      hasMaxPrefix = 1;
      paramName += 4;
    }

    for (; strlen(param->m_name) > 0; ++param)
    {
      if (wx_sqlite3_stricmp(paramName, param->m_name) == 0) break;
    }
    if (strlen(param->m_name) > 0)
    {
      if (db != NULL)
      {
        wx_sqlite3_mutex_enter(db->mutex);
      }
      else
      {
        wx_sqlite3_mutex_enter(wx_sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER));
      }
      value = (hasDefaultPrefix) ? param->m_default : (hasMinPrefix) ? param->m_minValue : (hasMaxPrefix) ? param->m_maxValue : param->m_value;
      if (!hasMinPrefix && !hasMaxPrefix && newValue >= 0 && newValue >= param->m_minValue && newValue <= param->m_maxValue)
      {
        if (hasDefaultPrefix)
        {
          param->m_default = newValue;
        }
        param->m_value = newValue;
        value = newValue;
      }
      if (db != NULL)
      {
        wx_sqlite3_mutex_leave(db->mutex);
      }
      else
      {
        wx_sqlite3_mutex_leave(wx_sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MASTER));
      }
    }
  }
  return value;
}

int
GetCipherType(wx_sqlite3* db)
{
  CodecParameter* codecParams = (db != NULL) ? GetCodecParams(db) : codecParameterTable;
  CipherParams* cipherParamTable = (codecParams != NULL) ? codecParams[0].m_params : commonParams;
#if 0
  if (codecParams == NULL)
  {
    return value;
  }

  int j = 0;
  for (j = 0; strlen(codecParams[j].m_name) > 0; ++j)
  {
    if (wx_sqlite3_stricmp(cipherName, codecParams[j].m_name) == 0) break;
  }
  if (strlen(codecParams[j].m_name) > 0)
  {
    cipherParamTable = codecParams[j].m_params;
  }
#endif

  int cipherType = CODEC_TYPE;
  CipherParams* cipher = cipherParamTable;
  for (; strlen(cipher->m_name) > 0; ++cipher)
  {
    if (wx_sqlite3_stricmp("cipher", cipher->m_name) == 0) break;
  }
  if (strlen(cipher->m_name) > 0)
  {
    cipherType = cipher->m_value;
    cipher->m_value = cipher->m_default;
  }
  return cipherType;
}

void*
GetCipherParams(wx_sqlite3* db, int cypherType)
{
  CodecParameter* codecParams = (db != NULL) ? GetCodecParams(db) : codecParameterTable;
  CipherParams* cipherParamTable = (codecParams != NULL) ? codecParams[cypherType].m_params : codecParameterTable[cypherType].m_params;
  return cipherParamTable;
}

int
CodecInit(Codec* codec)
{
  int rc = SQLITE_OK;
  if (codec != NULL)
  {
    codec->m_isEncrypted = 0;
    codec->m_hasReadCipher = 0;
    codec->m_readCipherType = CODEC_TYPE_UNKNOWN;
    codec->m_readCipher = NULL;
    codec->m_hasWriteCipher = 0;
    codec->m_writeCipherType = CODEC_TYPE_UNKNOWN;
    codec->m_writeCipher = NULL;
    codec->m_db = NULL;
    codec->m_bt = NULL;
    memset(codec->m_page, 0, sizeof(codec->m_page));
    codec->m_pageSize = 0;
    codec->m_reserved = 0;
  }
  else
  {
    rc = SQLITE_NOMEM;
  }
  return rc;
}

void
CodecTerm(Codec* codec)
{
  if (codec->m_readCipher != NULL)
  {
    codecDescriptorTable[codec->m_readCipherType-1].m_freeCipher(codec->m_readCipher);
    codec->m_readCipher = NULL;
  }
  if (codec->m_writeCipher != NULL)
  {
    codecDescriptorTable[codec->m_writeCipherType-1].m_freeCipher(codec->m_writeCipher);
    codec->m_writeCipher = NULL;
  }
  memset(codec, 0, sizeof(Codec));
}

int
CodecSetup(Codec* codec, int cipherType, char* userPassword, int passwordLength)
{
  int rc = SQLITE_OK;
  codec->m_isEncrypted = 1;
  codec->m_hasReadCipher = 1;
  codec->m_hasWriteCipher = 1;
  codec->m_readCipherType = cipherType;
  codec->m_readCipher = codecDescriptorTable[codec->m_readCipherType-1].m_allocateCipher(codec->m_db);
  if (codec->m_readCipher != NULL)
  {
    CodecGenerateReadKey(codec, userPassword, passwordLength);
    rc = CodecCopyCipher(codec, 1);
  }
  else
  {
    rc = SQLITE_NOMEM;
  }
  return rc;
}

int
CodecSetupWriteCipher(Codec* codec, int cipherType, char* userPassword, int passwordLength)
{
  int rc = SQLITE_OK;
  if (codec->m_writeCipher != NULL)
  {
    codecDescriptorTable[codec->m_writeCipherType-1].m_freeCipher(codec->m_writeCipher);
  }
  codec->m_isEncrypted = 1;
  codec->m_hasWriteCipher = 1;
  codec->m_writeCipherType = cipherType;
  codec->m_writeCipher = codecDescriptorTable[codec->m_writeCipherType-1].m_allocateCipher(codec->m_db);
  if (codec->m_writeCipher != NULL)
  {
    CodecGenerateWriteKey(codec, userPassword, passwordLength);
  }
  else
  {
    rc = SQLITE_NOMEM;
  }
  return rc;
}

void
CodecSetIsEncrypted(Codec* codec, int isEncrypted)
{
  codec->m_isEncrypted = isEncrypted;
}

void
CodecSetReadCipherType(Codec* codec, int cipherType)
{
  codec->m_readCipherType = cipherType;
}

void
CodecSetWriteCipherType(Codec* codec, int cipherType)
{
  codec->m_writeCipherType = cipherType;
}

void
CodecSetHasReadCipher(Codec* codec, int hasReadCipher)
{
  codec->m_hasReadCipher = hasReadCipher;
}

void
CodecSetHasWriteCipher(Codec* codec, int hasWriteCipher)
{
  codec->m_hasWriteCipher = hasWriteCipher;
}

void
CodecSetDb(Codec* codec, wx_sqlite3* db)
{
  codec->m_db = db;
}

void
CodecSetBtree(Codec* codec, Btree* bt)
{
  codec->m_bt = bt;
}

int
CodecIsEncrypted(Codec* codec)
{
  return codec->m_isEncrypted;
}

int
CodecHasReadCipher(Codec* codec)
{
  return codec->m_hasReadCipher;
}

int
CodecHasWriteCipher(Codec* codec)
{
  return codec->m_hasWriteCipher;
}

Btree*
CodecGetBtree(Codec* codec)
{
  return codec->m_bt;
}

unsigned char*
CodecGetPageBuffer(Codec* codec)
{
  return &codec->m_page[4];
}

int CodecGetLegacyReadCipher(Codec* codec)
{
  int legacy = (codec->m_hasReadCipher  && codec->m_readCipher != NULL) ? codecDescriptorTable[codec->m_readCipherType - 1].m_getLegacy(codec->m_readCipher) : 0;
  return legacy;
}

int CodecGetLegacyWriteCipher(Codec* codec)
{
  int legacy = (codec->m_hasWriteCipher && codec->m_writeCipher != NULL) ? codecDescriptorTable[codec->m_writeCipherType - 1].m_getLegacy(codec->m_writeCipher) : -1;
  return legacy;
}

int
CodecGetPageSizeReadCipher(Codec* codec)
{
  int pageSize = (codec->m_hasReadCipher  && codec->m_readCipher != NULL) ? codecDescriptorTable[codec->m_readCipherType - 1].m_getPageSize(codec->m_readCipher) : 0;
  return pageSize;
}

int
CodecGetPageSizeWriteCipher(Codec* codec)
{
  int pageSize = (codec->m_hasWriteCipher && codec->m_writeCipher != NULL) ? codecDescriptorTable[codec->m_writeCipherType - 1].m_getPageSize(codec->m_writeCipher) : -1;
  return pageSize;
}

int
CodecGetReservedReadCipher(Codec* codec)
{
  int reserved = (codec->m_hasReadCipher  && codec->m_readCipher != NULL) ? codecDescriptorTable[codec->m_readCipherType-1].m_getReserved(codec->m_readCipher) : -1;
  return reserved;
}

int
CodecGetReservedWriteCipher(Codec* codec)
{
  int reserved = (codec->m_hasWriteCipher && codec->m_writeCipher != NULL) ? codecDescriptorTable[codec->m_writeCipherType-1].m_getReserved(codec->m_writeCipher) : -1;
  return reserved;
}

int
CodecReservedEqual(Codec* codec)
{
  int readReserved  = (codec->m_hasReadCipher  && codec->m_readCipher  != NULL) ? codecDescriptorTable[codec->m_readCipherType-1].m_getReserved(codec->m_readCipher)   : -1;
  int writeReserved = (codec->m_hasWriteCipher && codec->m_writeCipher != NULL) ? codecDescriptorTable[codec->m_writeCipherType-1].m_getReserved(codec->m_writeCipher) : -1;
  return (readReserved == writeReserved);
}

int
CodecCopy(Codec* codec, Codec* other)
{
  int rc = SQLITE_OK;
  codec->m_isEncrypted = other->m_isEncrypted;
  codec->m_hasReadCipher = other->m_hasReadCipher;
  codec->m_hasWriteCipher = other->m_hasWriteCipher;
  codec->m_readCipherType = other->m_readCipherType;
  codec->m_writeCipherType = other->m_writeCipherType;
  codec->m_readCipher = NULL;
  codec->m_writeCipher = NULL;

  if (codec->m_hasReadCipher)
  {
    codec->m_readCipher = codecDescriptorTable[codec->m_readCipherType - 1].m_allocateCipher(codec->m_db);
    if (codec->m_readCipher != NULL)
    {
      codecDescriptorTable[codec->m_readCipherType - 1].m_cloneCipher(codec->m_readCipher, other->m_readCipher);
    }
    else
    {
      rc = SQLITE_NOMEM;
    }
  }

  if (codec->m_hasWriteCipher)
  {
    codec->m_writeCipher = codecDescriptorTable[codec->m_writeCipherType - 1].m_allocateCipher(codec->m_db);
    if (codec->m_writeCipher != NULL)
    {
      codecDescriptorTable[codec->m_writeCipherType - 1].m_cloneCipher(codec->m_writeCipher, other->m_writeCipher);
    }
    else
    {
      rc = SQLITE_NOMEM;
    }
  }
  codec->m_db = other->m_db;
  codec->m_bt = other->m_bt;
  return rc;
}

int
CodecCopyCipher(Codec* codec, int read2write)
{
  int rc = SQLITE_OK;
  if (read2write)
  {
    if (codec->m_writeCipher != NULL && codec->m_writeCipherType != codec->m_readCipherType)
    {
      codecDescriptorTable[codec->m_writeCipherType-1].m_freeCipher(codec->m_writeCipher);
      codec->m_writeCipher = NULL;
    }
    if (codec->m_writeCipher == NULL)
    {
      codec->m_writeCipherType = codec->m_readCipherType;
      codec->m_writeCipher = codecDescriptorTable[codec->m_writeCipherType-1].m_allocateCipher(codec->m_db);
    }
    if (codec->m_writeCipher != NULL)
    {
      codecDescriptorTable[codec->m_writeCipherType-1].m_cloneCipher(codec->m_writeCipher, codec->m_readCipher);
    }
    else
    {
      rc = SQLITE_NOMEM;
    }
  }
  else
  {
    if (codec->m_readCipher != NULL && codec->m_readCipherType != codec->m_writeCipherType)
    {
      codecDescriptorTable[codec->m_readCipherType-1].m_freeCipher(codec->m_readCipher);
      codec->m_readCipher = NULL;
    }
    if (codec->m_readCipher == NULL)
    {
      codec->m_readCipherType = codec->m_writeCipherType;
      codec->m_readCipher = codecDescriptorTable[codec->m_readCipherType-1].m_allocateCipher(codec->m_db);
    }
    if (codec->m_readCipher != NULL)
    {
      codecDescriptorTable[codec->m_readCipherType-1].m_cloneCipher(codec->m_readCipher, codec->m_writeCipher);
    }
    else
    {
      rc = SQLITE_NOMEM;
    }
  }
  return rc;
}

void
CodecPadPassword(char* password, int pswdlen, unsigned char pswd[32])
{
  int j;
  int p = 0;
  int m = pswdlen;
  if (m > 32) m = 32;

  for (j = 0; j < m; j++)
  {
    pswd[p++] = (unsigned char) password[j];
  }
  for (j = 0; p < 32 && j < 32; j++)
  {
    pswd[p++] = padding[j];
  }
}

void
CodecGenerateReadKey(Codec* codec, char* userPassword, int passwordLength)
{
  codecDescriptorTable[codec->m_readCipherType-1].m_generateKey(codec->m_readCipher, codec->m_bt, userPassword, passwordLength, 0);
}

void
CodecGenerateWriteKey(Codec* codec, char* userPassword, int passwordLength)
{
  codecDescriptorTable[codec->m_writeCipherType-1].m_generateKey(codec->m_writeCipher, codec->m_bt, userPassword, passwordLength, 1);
}

int
CodecEncrypt(Codec* codec, int page, unsigned char* data, int len, int useWriteKey)
{
  int cipherType = (useWriteKey) ? codec->m_writeCipherType : codec->m_readCipherType;
  void* cipher = (useWriteKey) ? codec->m_writeCipher : codec->m_readCipher;
  return codecDescriptorTable[cipherType-1].m_encryptPage(cipher, page, data, len, codec->m_reserved);
}

int
CodecDecrypt(Codec* codec, int page, unsigned char* data, int len)
{
  int cipherType = codec->m_readCipherType;
  void* cipher = codec->m_readCipher;
  return codecDescriptorTable[cipherType-1].m_decryptPage(cipher, page, data, len, codec->m_reserved);
}

