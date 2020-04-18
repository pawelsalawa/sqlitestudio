/*
** Name:        wx_sqlite3secure.h
** Purpose:     Header file for SQLite codecs
** Author:      Ulrich Telle
** Created:     2018-03-30
** Copyright:   (c) 2018 Ulrich Telle
** License:     LGPL-3.0+ WITH WxWindows-exception-3.1
*/

#ifndef SQLITE3SECURE_H_
#define SQLITE3SECURE_H_

#include "wxsqlite3.h"

// Symbols for ciphers
#define CODEC_TYPE_UNKNOWN   0
#define CODEC_TYPE_AES128    1
#define CODEC_TYPE_AES256    2
#define CODEC_TYPE_CHACHA20  3
#define CODEC_TYPE_SQLCIPHER 4
#define CODEC_TYPE_MAX       4

#ifdef WXSQLITE3_HAVE_CODEC
#undef SQLITE_HAS_CODEC
#define SQLITE_HAS_CODEC WXSQLITE3_HAVE_CODEC
#endif

#ifndef SQLITE_HAS_CODEC
#define SQLITE_HAS_CODEC 1
#endif

// Define default cipher support
#ifdef WXSQLITE3_HAVE_CIPHER_AES_128_CBC
#define HAVE_CIPHER_AES_128_CBC WXSQLITE3_HAVE_CIPHER_AES_128_CBC
#endif

#ifndef HAVE_CIPHER_AES_128_CBC
#define HAVE_CIPHER_AES_128_CBC 1
#endif

#ifdef WXSQLITE3_HAVE_CIPHER_AES_256_CBC
#define HAVE_CIPHER_AES_256_CBC WXSQLITE3_HAVE_CIPHER_AES_256_CBC
#endif

#ifndef HAVE_CIPHER_AES_256_CBC
#define HAVE_CIPHER_AES_256_CBC 1
#endif

#ifdef WXSQLITE3_HAVE_CIPHER_CHACHA20
#define HAVE_CIPHER_CHACHA20 WXSQLITE3_HAVE_CIPHER_CHACHA20
#endif

#ifndef HAVE_CIPHER_CHACHA20
#define HAVE_CIPHER_CHACHA20 1
#endif

#ifdef WXSQLITE3_HAVE_CIPHER_SQLCIPHER
#define HAVE_CIPHER_SQLCIPHER WXSQLITE3_HAVE_CIPHER_SQLCIPHER
#endif

#ifndef HAVE_CIPHER_SQLCIPHER
#define HAVE_CIPHER_SQLCIPHER 1
#endif

// Define Windows specific SQLite API functions (not defined in wxsqlite3.h)
#if SQLITE_VERSION_NUMBER >= 3007014
#if SQLITE_OS_WIN == 1
#ifdef __cplusplus
extern "C" {
#endif
#if SQLITE_VERSION_NUMBER >= 3024000
SQLITE_API int wx_sqlite3_win32_set_directory(unsigned long type, void* zValue);
#else
SQLITE_API int wx_sqlite3_win32_set_directory(DWORD type, LPCWSTR zValue);
#endif
#ifdef __cplusplus
}
#endif
#endif
#endif

#ifdef SQLITE_HAS_CODEC

// Define functions for the configuration of the wxSQLite3 encryption extension
#ifdef __cplusplus
extern "C" {
#endif
SQLITE_API int wxwx_sqlite3_config(wx_sqlite3* db, const char* paramName, int newValue);
SQLITE_API int wxwx_sqlite3_config_cipher(wx_sqlite3* db, const char* cipherName, const char* paramName, int newValue);
SQLITE_API unsigned char* wxwx_sqlite3_codec_data(wx_sqlite3* db, const char* zDbName, const char* paramName);
#ifdef __cplusplus
}

#endif

#endif

#endif

