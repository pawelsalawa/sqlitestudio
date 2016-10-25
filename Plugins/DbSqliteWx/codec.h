/*
///////////////////////////////////////////////////////////////////////////////
// Name:        codec.h
// Purpose:     
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-12-06
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file codec.h Interface of the codec class
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

#define CODEC_TYPE_AES128 1
#define CODEC_TYPE_AES256 2

#ifndef CODEC_TYPE
#define CODEC_TYPE CODEC_TYPE_AES128
#endif

#if CODEC_TYPE == CODEC_TYPE_AES256
#define KEYLENGTH 32
#define CODEC_SHA_ITER 4001
#else
#define KEYLENGTH 16
#endif

typedef struct _Codec
{
  int           m_isEncrypted;
  int           m_hasReadKey;
  unsigned char m_readKey[KEYLENGTH];
  int           m_hasWriteKey;
  unsigned char m_writeKey[KEYLENGTH];
  Rijndael*     m_aes;

  Btree*        m_bt; /* Pointer to B-tree used by DB */
  unsigned char m_page[SQLITE_MAX_PAGE_SIZE+24];
} Codec;

void CodecInit(Codec* codec);
void CodecTerm(Codec* codec);

void CodecCopy(Codec* codec, Codec* other);

void CodecGenerateReadKey(Codec* codec, char* userPassword, int passwordLength);

void CodecGenerateWriteKey(Codec* codec, char* userPassword, int passwordLength);

void CodecEncrypt(Codec* codec, int page, unsigned char* data, int len, int useWriteKey);

void CodecDecrypt(Codec* codec, int page, unsigned char* data, int len);

void CodecCopyKey(Codec* codec, int read2write);

void CodecSetIsEncrypted(Codec* codec, int isEncrypted);
void CodecSetHasReadKey(Codec* codec, int hasReadKey);
void CodecSetHasWriteKey(Codec* codec, int hasWriteKey);
void CodecSetBtree(Codec* codec, Btree* bt);

int CodecIsEncrypted(Codec* codec);
int CodecHasReadKey(Codec* codec);
int CodecHasWriteKey(Codec* codec);
Btree* CodecGetBtree(Codec* codec);
unsigned char* CodecGetPageBuffer(Codec* codec);

void CodecGenerateEncryptionKey(Codec* codec, char* userPassword, int passwordLength, 
                                unsigned char encryptionKey[KEYLENGTH]);

void CodecPadPassword(Codec* codec, char* password, int pswdlen, unsigned char pswd[32]);

void CodecRC4(Codec* codec, unsigned char* key, int keylen,
              unsigned char* textin, int textlen,
         unsigned char* textout);

void CodecGetMD5Binary(Codec* codec, unsigned char* data, int length, unsigned char* digest);

#if CODEC_TYPE == CODEC_TYPE_AES256
void CodecGetSHABinary(Codec* codec, unsigned char* data, int length, unsigned char* digest);
#endif
  
void CodecGenerateInitialVector(Codec* codec, int seed, unsigned char iv[16]);

void CodecAES(Codec* codec, int page, int encrypt, unsigned char encryptionKey[KEYLENGTH],
              unsigned char* datain, int datalen, unsigned char* dataout);

#endif

