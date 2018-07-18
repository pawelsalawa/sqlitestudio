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

// Define Windows specific SQLite API functions (not defined in wxsqlite3.h)
#if SQLITE_VERSION_NUMBER >= 3007014
#if SQLITE_OS_WIN == 1
#ifdef __cplusplus
extern "C" {
#endif
SQLITE_API int wx_sqlite3_win32_set_directory(DWORD type, LPCWSTR zValue);
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
#ifdef __cplusplus
}

#endif

#endif

#endif

