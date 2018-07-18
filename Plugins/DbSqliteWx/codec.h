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

#define CODEC_TYPE_UNKNOWN   0
#define CODEC_TYPE_AES128    1
#define CODEC_TYPE_AES256    2
#define CODEC_TYPE_CHACHA20  3
#define CODEC_TYPE_SQLCIPHER 4
#define CODEC_TYPE_MAX       4

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

#define CODEC_SHA_ITER 4001

typedef struct _Codec
{
  int           m_isEncrypted;
  /* Read cipher */
  int           m_hasReadCipher;
  int           m_readCipherType;
  void*         m_readCipher;
  /* Write cipher */
  int           m_hasWriteCipher;
  int           m_writeCipherType;
  void*         m_writeCipher;

  wx_sqlite3*      m_db; /* Pointer to DB */
  Btree*        m_bt; /* Pointer to B-tree used by DB */
  unsigned char m_page[SQLITE_MAX_PAGE_SIZE+24];
  int           m_pageSize;
  int           m_reserved;
} Codec;

void wxwx_sqlite3_config_table(wx_sqlite3_context* context, int argc, wx_sqlite3_value** argv);
void wxwx_sqlite3_config_params(wx_sqlite3_context* context, int argc, wx_sqlite3_value** argv);

int wxwx_sqlite3_config(wx_sqlite3* db, const char* paramName, int newValue);
int wxwx_sqlite3_config_cipher(wx_sqlite3* db, const char* cipherName, const char* paramName, int newValue);

int GetCipherType(wx_sqlite3* db);
void* GetCipherParams(wx_sqlite3* db, int cypherType);
int CodecInit(Codec* codec);
void CodecTerm(Codec* codec);

int CodecCopy(Codec* codec, Codec* other);

void CodecGenerateReadKey(Codec* codec, char* userPassword, int passwordLength);

void CodecGenerateWriteKey(Codec* codec, char* userPassword, int passwordLength);

int CodecEncrypt(Codec* codec, int page, unsigned char* data, int len, int useWriteKey);

int CodecDecrypt(Codec* codec, int page, unsigned char* data, int len);

int CodecCopyCipher(Codec* codec, int read2write);

int CodecSetup(Codec* codec, int cipherType, char* userPassword, int passwordLength);
int CodecSetupWriteCipher(Codec* codec, int cipherType, char* userPassword, int passwordLength);

void CodecSetIsEncrypted(Codec* codec, int isEncrypted);
void CodecSetReadCipherType(Codec* codec, int cipherType);
void CodecSetWriteCipherType(Codec* codec, int cipherType);
void CodecSetHasReadCipher(Codec* codec, int hasReadCipher);
void CodecSetHasWriteCipher(Codec* codec, int hasWriteCipher);
void CodecSetDb(Codec* codec, wx_sqlite3* db);
void CodecSetBtree(Codec* codec, Btree* bt);

int CodecIsEncrypted(Codec* codec);
int CodecHasReadCipher(Codec* codec);
int CodecHasWriteCipher(Codec* codec);
Btree* CodecGetBtree(Codec* codec);
unsigned char* CodecGetPageBuffer(Codec* codec);
int CodecGetLegacyReadCipher(Codec* codec);
int CodecGetLegacyWriteCipher(Codec* codec);
int CodecGetPageSizeReadCipher(Codec* codec);
int CodecGetPageSizeWriteCipher(Codec* codec);
int CodecGetReservedReadCipher(Codec* codec);
int CodecGetReservedWriteCipher(Codec* codec);
int CodecReservedEqual(Codec* codec);

void CodecPadPassword(char* password, int pswdlen, unsigned char pswd[32]);
void CodecRC4(unsigned char* key, int keylen,
              unsigned char* textin, int textlen,
              unsigned char* textout);
void CodecGetMD5Binary(unsigned char* data, int length, unsigned char* digest);
void CodecGetSHABinary(unsigned char* data, int length, unsigned char* digest);
void CodecGenerateInitialVector(int seed, unsigned char iv[16]);

#endif

