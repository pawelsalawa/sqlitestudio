/*
** Name:        codec.h
** Purpose:     Header file for SQLite codecs
** Author:      Ulrich Telle
** Created:     2006-12-06
** Copyright:   (c) 2006-2018 Ulrich Telle
** License:     LGPL-3.0+ WITH WxWindows-exception-3.1
*/

#ifndef _CODEC_H_
#define _CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__BORLANDC__)
#define __STDC__ 1
#endif

#if defined(__BORLANDC__)
#undef __STDC__
#endif

/*
// ATTENTION: Macro similar to that in pager.c
// TODO: Check in case of new version of SQLite
*/
#define WX_PAGER_MJ_PGNO(x) ((PENDING_BYTE/(x))+1)

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#include "rijndael.h"

#include "sqlite3secure.h"

#define CODEC_TYPE_DEFAULT CODEC_TYPE_CHACHA20

#ifndef CODEC_TYPE
#define CODEC_TYPE CODEC_TYPE_DEFAULT
#endif

#if CODEC_TYPE < 1 || CODEC_TYPE > CODEC_TYPE_MAX
#error "Invalid codec type selected"
#endif

#define MAXKEYLENGTH     32
#define KEYLENGTH_AES128 16
#define KEYLENGTH_AES256 32
#define KEYSALT_LENGTH   16

#define CODEC_SHA_ITER 4001

typedef struct _Codec
{
  int           m_isEncrypted;
  int           m_hmacCheck;
  /* Read cipher */
  int           m_hasReadCipher;
  int           m_readCipherType;
  void*         m_readCipher;
  int           m_readReserved;
  /* Write cipher */
  int           m_hasWriteCipher;
  int           m_writeCipherType;
  void*         m_writeCipher;
  int           m_writeReserved;

  wx_sqlite3*      m_db; /* Pointer to DB */
  Btree*        m_bt; /* Pointer to B-tree used by DB */
  BtShared*     m_btShared; /* Pointer to shared B-tree used by DB */
  unsigned char m_page[SQLITE_MAX_PAGE_SIZE+24];
  int           m_pageSize;
  int           m_reserved;
  int           m_hasKeySalt;
  unsigned char m_keySalt[KEYSALT_LENGTH];
} Codec;

static void wxwx_sqlite3_config_table(wx_sqlite3_context* context, int argc, wx_sqlite3_value** argv);
static void wxwx_sqlite3_config_params(wx_sqlite3_context* context, int argc, wx_sqlite3_value** argv);

int wxwx_sqlite3_config(wx_sqlite3* db, const char* paramName, int newValue);
int wxwx_sqlite3_config_cipher(wx_sqlite3* db, const char* cipherName, const char* paramName, int newValue);

static int GetCipherType(wx_sqlite3* db);
static void* GetCipherParams(wx_sqlite3* db, int cypherType);
static int CodecInit(Codec* codec);
static void CodecTerm(Codec* codec);
static void CodecClearKeySalt(Codec* codec);

static int CodecCopy(Codec* codec, Codec* other);

static void CodecGenerateReadKey(Codec* codec, char* userPassword, int passwordLength, unsigned char* cipherSalt);

static void CodecGenerateWriteKey(Codec* codec, char* userPassword, int passwordLength, unsigned char* cipherSalt);

static int CodecEncrypt(Codec* codec, int page, unsigned char* data, int len, int useWriteKey);

static int CodecDecrypt(Codec* codec, int page, unsigned char* data, int len);

static int CodecCopyCipher(Codec* codec, int read2write);

static int CodecSetup(Codec* codec, int cipherType, char* userPassword, int passwordLength);
static int CodecSetupWriteCipher(Codec* codec, int cipherType, char* userPassword, int passwordLength);

static void CodecSetIsEncrypted(Codec* codec, int isEncrypted);
static void CodecSetReadCipherType(Codec* codec, int cipherType);
static void CodecSetWriteCipherType(Codec* codec, int cipherType);
static void CodecSetHasReadCipher(Codec* codec, int hasReadCipher);
static void CodecSetHasWriteCipher(Codec* codec, int hasWriteCipher);
static void CodecSetDb(Codec* codec, wx_sqlite3* db);
static void CodecSetBtree(Codec* codec, Btree* bt);
static void CodecSetReadReserved(Codec* codec, int reserved);
static void CodecSetWriteReserved(Codec* codec, int reserved);

static int CodecIsEncrypted(Codec* codec);
static int CodecHasReadCipher(Codec* codec);
static int CodecHasWriteCipher(Codec* codec);
static Btree* CodecGetBtree(Codec* codec);
static BtShared* CodecGetBtShared(Codec* codec);
static int CodecGetPageSize(Codec* codec);
static int CodecGetReadReserved(Codec* codec);
static int CodecGetWriteReserved(Codec* codec);
static unsigned char* CodecGetPageBuffer(Codec* codec);
static int CodecGetLegacyReadCipher(Codec* codec);
static int CodecGetLegacyWriteCipher(Codec* codec);
static int CodecGetPageSizeReadCipher(Codec* codec);
static int CodecGetPageSizeWriteCipher(Codec* codec);
static int CodecGetReservedReadCipher(Codec* codec);
static int CodecGetReservedWriteCipher(Codec* codec);
static int CodecReservedEqual(Codec* codec);

static void CodecPadPassword(char* password, int pswdlen, unsigned char pswd[32]);
static void CodecRC4(unsigned char* key, int keylen,
                     unsigned char* textin, int textlen,
                     unsigned char* textout);
static void CodecGetMD5Binary(unsigned char* data, int length, unsigned char* digest);
static void CodecGetSHABinary(unsigned char* data, int length, unsigned char* digest);
static void CodecGenerateInitialVector(int seed, unsigned char iv[16]);

#endif

