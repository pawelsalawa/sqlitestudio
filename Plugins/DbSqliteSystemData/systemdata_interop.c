/********************************************************
 * ADO.NET 2.0 Data Provider for SQLite Version 3.X
 * Written by Robert Simpson (robert@blackcastlesoft.com)
 *
 * Released to the public domain, use at your own risk!
 ********************************************************/

#ifdef _WIN32
#define SQLITE_API __declspec(dllexport)
#else
#define WINAPI
#endif

#if /* SQLITE_VERSION_NUMBER >= 3013000 && */ defined(INTEROP_SESSION_EXTENSION)
#ifndef SQLITE_ENABLE_SESSION
#define SQLITE_ENABLE_SESSION (1)
#endif
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
#define SQLITE_ENABLE_PREUPDATE_HOOK (1)
#endif
#endif

#include "systemdata_sqlite3.c"

#if !SQLITE_OS_WIN
#include <wchar.h>
#endif

#if defined(INTEROP_INCLUDE_EXTRA)
#include "../ext/extra.c"
#endif

#if defined(INTEROP_INCLUDE_CEROD)
#include "../ext/cerod.c"
#endif

#if defined(INTEROP_INCLUDE_SEE)
#include "../ext/see.c"
#endif

#if defined(INTEROP_INCLUDE_ZIPVFS)
#include "../ext/zipvfs.c"
#include "../ext/algorithms.c"
#endif

#if defined(INTEROP_EXTENSION_FUNCTIONS)
#undef COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE
#include "systemdata_extension-functions.c"
extern int RegisterExtensionFunctions(systemdata_sqlite3 *db);
#endif

#if SQLITE_OS_WIN && defined(INTEROP_CODEC) && !defined(INTEROP_INCLUDE_SEE)
#ifdef SQLITE_ENABLE_ZIPVFS
#define INTEROP_CODEC_GET_PAGER(a,b,c) systemdata_sqlite3PagerGet(a,b,c,0)
#elif SQLITE_VERSION_NUMBER >= 3010000
#define INTEROP_CODEC_GET_PAGER(a,b,c) systemdata_sqlite3PagerGet(a,b,c,0)
#else
#define INTEROP_CODEC_GET_PAGER(a,b,c) systemdata_sqlite3PagerGet(a,b,c)
#endif
#include "systemdata_crypt.c"
#endif

#include "systemdata_interop.h"

#define INTEROP_DEBUG_NONE           (0x0000)
#define INTEROP_DEBUG_CLOSE          (0x0001)
#define INTEROP_DEBUG_FINALIZE       (0x0002)
#define INTEROP_DEBUG_BACKUP_FINISH  (0x0004)
#define INTEROP_DEBUG_OPEN           (0x0008)
#define INTEROP_DEBUG_OPEN16         (0x0010)
#define INTEROP_DEBUG_PREPARE        (0x0020)
#define INTEROP_DEBUG_PREPARE16      (0x0040)
#define INTEROP_DEBUG_RESET          (0x0080)
#define INTEROP_DEBUG_CHANGES        (0x0100)
#define INTEROP_DEBUG_BREAK          (0x0200)
#define INTEROP_DEBUG_BLOB_CLOSE     (0x0400)

#if defined(_MSC_VER) && defined(INTEROP_DEBUG) && \
    (INTEROP_DEBUG & INTEROP_DEBUG_BREAK)
#define systemdata_sqlite3InteropBreak(a) { systemdata_sqlite3InteropDebug("%s\n", (a)); __debugbreak(); }
#else
#define systemdata_sqlite3InteropBreak(a)
#endif

typedef void (*SQLITEUSERFUNC)(systemdata_sqlite3_context *, int, systemdata_sqlite3_value **);
typedef void (*SQLITEFUNCFINAL)(systemdata_sqlite3_context *);

/*
** An array of names of all compile-time options.  This array should
** be sorted A-Z.
**
** This array looks large, but in a typical installation actually uses
** only a handful of compile-time options, so most times this array is usually
** rather short and uses little memory space.
*/
static const char * const azInteropCompileOpt[] = {

/* These macros are provided to "stringify" the value of the define
** for those options in which the value is meaningful. */
#ifndef CTIMEOPT_VAL_
#define CTIMEOPT_VAL_(opt) #opt
#endif

#ifndef CTIMEOPT_VAL
#define CTIMEOPT_VAL(opt) CTIMEOPT_VAL_(opt)
#endif

#ifdef INTEROP_CODEC
  "CODEC",
#endif
#ifdef INTEROP_DEBUG
  "DEBUG=" CTIMEOPT_VAL(INTEROP_DEBUG),
#endif
#ifdef INTEROP_EXTENSION_FUNCTIONS
  "EXTENSION_FUNCTIONS",
#endif
#ifdef INTEROP_INCLUDE_CEROD
  "INCLUDE_CEROD",
#endif
#ifdef INTEROP_INCLUDE_EXTRA
  "INCLUDE_EXTRA",
#endif
#ifdef INTEROP_INCLUDE_SEE
  "INCLUDE_SEE",
#endif
#ifdef INTEROP_INCLUDE_ZIPVFS
  "INCLUDE_ZIPVFS",
#endif
#ifdef INTEROP_JSON1_EXTENSION
  "JSON1_EXTENSION",
#endif
#ifdef INTEROP_LEGACY_CLOSE
  "LEGACY_CLOSE",
#endif
#ifdef INTEROP_LOG
  "LOG",
#endif
#ifdef INTEROP_PERCENTILE_EXTENSION
  "PERCENTILE_EXTENSION",
#endif
#ifdef INTEROP_REGEXP_EXTENSION
  "REGEXP_EXTENSION",
#endif
#ifdef INTEROP_SESSION_EXTENSION
  "SESSION_EXTENSION",
#endif
#ifdef INTEROP_SHA1_EXTENSION
  "SHA1_EXTENSION",
#endif
#ifdef INTEROP_TEST_EXTENSION
  "TEST_EXTENSION",
#endif
#ifdef INTEROP_TOTYPE_EXTENSION
  "TOTYPE_EXTENSION",
#endif
#ifdef SQLITE_VERSION_NUMBER
  "VERSION_NUMBER=" CTIMEOPT_VAL(SQLITE_VERSION_NUMBER),
#endif
#ifdef INTEROP_VIRTUAL_TABLE
  "VIRTUAL_TABLE",
#endif
};

/*
** Given the name of a compile-time option, return true if that option
** was used and false if not.
**
** The name can optionally begin with "SQLITE_" or "INTEROP_" but those
** prefixes are not required for a match.
*/
SQLITE_API int WINAPI interop_compileoption_used(const char *zOptName){
  int i, n;
  if( systemdata_sqlite3StrNICmp(zOptName, "SQLITE_", 7)==0 ) zOptName += 7;
  if( systemdata_sqlite3StrNICmp(zOptName, "INTEROP_", 8)==0 ) zOptName += 8;
  n = systemdata_sqlite3Strlen30(zOptName);

  /* Since ArraySize(azInteropCompileOpt) is normally in single digits, a
  ** linear search is adequate.  No need for a binary search. */
  for(i=0; i<ArraySize(azInteropCompileOpt); i++){
    if( systemdata_sqlite3StrNICmp(zOptName, azInteropCompileOpt[i], n)==0
     && systemdata_sqlite3CtypeMap[(unsigned char)azInteropCompileOpt[i][n]]==0
    ){
      return 1;
    }
  }
  return 0;
}

/*
** Return the N-th compile-time option string.  If N is out of range,
** return a NULL pointer.
*/
SQLITE_API const char *WINAPI interop_compileoption_get(int N){
  if( N>=0 && N<ArraySize(azInteropCompileOpt) ){
    return azInteropCompileOpt[N];
  }
  return 0;
}

#if defined(INTEROP_DEBUG) || defined(INTEROP_LOG)
SQLITE_PRIVATE void systemdata_sqlite3InteropDebug(const char *zFormat, ...){
  va_list ap;                         /* Vararg list */
  StrAccum acc;                       /* String accumulator */
  char zMsg[SQLITE_PRINT_BUF_SIZE*3]; /* Complete log message */
  va_start(ap, zFormat);
#if SQLITE_VERSION_NUMBER >= 3008010
  systemdata_sqlite3StrAccumInit(&acc, 0, zMsg, sizeof(zMsg), 0);
#else
  systemdata_sqlite3StrAccumInit(&acc, zMsg, sizeof(zMsg), 0);
  acc.useMalloc = 0;
#endif
#if SQLITE_VERSION_NUMBER >= 3011000
  systemdata_sqlite3VXPrintf(&acc, zFormat, ap);
#else
  systemdata_sqlite3VXPrintf(&acc, 0, zFormat, ap);
#endif
  va_end(ap);
#if SQLITE_OS_WIN && SQLITE_VERSION_NUMBER >= 3007013
  systemdata_sqlite3_win32_write_debug(systemdata_sqlite3StrAccumFinish(&acc), -1);
#elif SQLITE_OS_WIN && defined(SQLITE_WIN32_HAS_ANSI)
  OutputDebugStringA(systemdata_sqlite3StrAccumFinish(&acc));
#elif SQLITE_OS_WIN && defined(SQLITE_WIN32_HAS_WIDE)
  {
    LPWSTR zWideMsg = utf8ToUnicode(systemdata_sqlite3StrAccumFinish(&acc));
    if( zWideMsg ){
      OutputDebugStringW(zWideMsg);
      systemdata_sqlite3_free(zWideMsg);
    }
  }
#else
  fprintf(stderr, "%s", systemdata_sqlite3StrAccumFinish(&acc));
#endif
}
#endif

#if defined(INTEROP_LOG)
SQLITE_PRIVATE int logConfigured = 0;

SQLITE_PRIVATE void systemdata_sqlite3InteropLogCallback(void *pArg, int iCode, const char *zMsg){
  systemdata_sqlite3InteropDebug("INTEROP_LOG (%d) %s\n", iCode, zMsg);
}
#endif

SQLITE_API int WINAPI systemdata_sqlite3_malloc_size_interop(void *p){
  return systemdata_sqlite3MallocSize(p);
}

#if defined(INTEROP_LEGACY_CLOSE) || SQLITE_VERSION_NUMBER < 3007014
SQLITE_PRIVATE void * systemdata_sqlite3DbMallocZero_interop(systemdata_sqlite3 *db, int n)
{
  void *p;
  if (db) {
    systemdata_sqlite3_mutex_enter(db->mutex);
  }
  p = systemdata_sqlite3DbMallocZero(db,n);
  if (db) {
    systemdata_sqlite3_mutex_leave(db->mutex);
  }
  return p;
}

SQLITE_PRIVATE void systemdata_sqlite3DbFree_interop(systemdata_sqlite3 *db, void *p)
{
  if (db) {
    systemdata_sqlite3_mutex_enter(db->mutex);
  }
  if (p) {
    systemdata_sqlite3MemdebugSetType(p, MEMTYPE_DB|MEMTYPE_HEAP);
  }
  systemdata_sqlite3DbFree(db,p);
  if (db) {
    systemdata_sqlite3_mutex_leave(db->mutex);
  }
}
#endif

/*
    The goal of this version of close is different than that of systemdata_sqlite3_close(), and is designed to lend itself better to .NET's non-deterministic finalizers and
    the GC thread.  SQLite will not close a database if statements are open on it -- but for our purposes, we'd rather finalize all active statements
    and forcibly close the database.  The reason is simple -- a lot of people don't Dispose() of their objects correctly and let the garbage collector
    do it.  This leads to unexpected behavior when a user thinks they've closed a database, but it's still open because not all the statements have
    hit the GC yet.

    So, here we have a problem ... .NET has a pointer to any number of systemdata_sqlite3_stmt objects.  We can't call systemdata_sqlite3_finalize() on these because
    their memory is freed and can be used for something else.  The GC thread could potentially try and call finalize again on the statement after
    that memory was deallocated.  BAD.  So, what we need to do is make a copy of each statement, and call finalize() on the copy -- so that the original
    statement's memory is preserved, and marked as BAD, but we can still manage to finalize everything and forcibly close the database.  Later when the
    GC gets around to calling finalize_interop() on the "bad" statement, we detect that and finish deallocating the pointer.
*/
SQLITE_API int WINAPI systemdata_sqlite3_close_interop(systemdata_sqlite3 *db)
{
  int ret;
#if !defined(INTEROP_LEGACY_CLOSE) && SQLITE_VERSION_NUMBER >= 3007014

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_CLOSE)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_close_interop(): calling systemdata_sqlite3_close_v2(%p)...\n", db);
#endif

  ret = systemdata_sqlite3_close_v2(db);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_CLOSE)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_close_interop(): systemdata_sqlite3_close_v2(%p) returned %d.\n", db, ret);
#endif

  return ret;
#else
  ret = systemdata_sqlite3_close(db);

  if (ret == SQLITE_BUSY)
  {
    systemdata_sqlite3_mutex_enter(db->mutex);

    if (!db->pVdbe)
    {
      systemdata_sqlite3_mutex_leave(db->mutex);
      return ret;
    }

    while (db->pVdbe)
    {
      /* Make a copy of the first prepared statement */
      Vdbe *p = (Vdbe *)systemdata_sqlite3DbMallocZero_interop(db, sizeof(Vdbe));
      Vdbe *po = db->pVdbe;

      if (!p)
      {
        ret = SQLITE_NOMEM;
        break;
      }

      CopyMemory(p, po, sizeof(Vdbe));

      /* Put it on the chain so we can free it */
      db->pVdbe = p;
      ret = systemdata_sqlite3_finalize((systemdata_sqlite3_stmt *)p); /* This will also free the copy's memory */
      if (ret)
      {
        /* finalize failed -- so we must put back anything we munged */
        CopyMemory(po, p, sizeof(Vdbe));
        db->pVdbe = po;

        /*
        ** NOTE: Ok, we must free this block that *we* allocated (above) since
        **       finalize did not do so.
        */
        systemdata_sqlite3DbFree_interop(db, p);
        break;
      }
      else
      {
        ZeroMemory(po, sizeof(Vdbe));
        po->magic = VDBE_MAGIC_DEAD;
      }
    }
    systemdata_sqlite3_mutex_leave(db->mutex);
    ret = systemdata_sqlite3_close(db);
  }
  return ret;
#endif
}

#if defined(INTEROP_LOG)
SQLITE_API int WINAPI systemdata_sqlite3_config_log_interop()
{
  int ret;
  if( !logConfigured ){
    ret = systemdata_sqlite3_config(SQLITE_CONFIG_LOG, systemdata_sqlite3InteropLogCallback, 0);
    if( ret==SQLITE_OK ){
      logConfigured = 1;
    }else{
      systemdata_sqlite3InteropDebug("systemdata_sqlite3_config_log_interop(): systemdata_sqlite3_config(SQLITE_CONFIG_LOG) returned %d.\n", ret);
    }
  }else{
    ret = SQLITE_OK;
  }
  return ret;
}
#endif

SQLITE_API const char *WINAPI interop_libversion(void)
{
  return INTEROP_VERSION;
}

SQLITE_API const char *WINAPI interop_sourceid(void)
{
  return INTEROP_SOURCE_ID " " INTEROP_SOURCE_TIMESTAMP;
}

SQLITE_API int WINAPI systemdata_sqlite3_open_interop(const char *filename, const char *vfsName, int flags, int extFuncs, systemdata_sqlite3 **ppdb)
{
  int ret;

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_OPEN)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_open_interop(): calling systemdata_sqlite3_open_v2(\"%s\", \"%s\", %d, %d, %p)...\n", filename, vfsName, flags, extFuncs, ppdb);
#endif

  ret = systemdata_sqlite3_open_v2(filename, ppdb, flags, vfsName);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_OPEN)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_open_interop(): systemdata_sqlite3_open_v2(\"%s\", \"%s\", %d, %d, %p) returned %d.\n", filename, vfsName, flags, extFuncs, ppdb, ret);
#endif

#if defined(INTEROP_EXTENSION_FUNCTIONS)
  if ((ret == SQLITE_OK) && ppdb && extFuncs)
    RegisterExtensionFunctions(*ppdb);
#endif

  return ret;
}

SQLITE_API int WINAPI systemdata_sqlite3_open16_interop(const char *filename, const char *vfsName, int flags, int extFuncs, systemdata_sqlite3 **ppdb)
{
  int ret;

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_OPEN16)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_open16_interop(): calling systemdata_sqlite3_open_interop(\"%s\", \"%s\", %d, %d, %p)...\n", filename, vfsName, flags, extFuncs, ppdb);
#endif

  ret = systemdata_sqlite3_open_interop(filename, vfsName, flags, extFuncs, ppdb);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_OPEN16)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_open16_interop(): systemdata_sqlite3_open_interop(\"%s\", \"%s\", %d, %d, %p) returned %d.\n", filename, vfsName, flags, extFuncs, ppdb, ret);
#endif

  if ((ret == SQLITE_OK) && ppdb && !DbHasProperty(*ppdb, 0, DB_SchemaLoaded))
  {
    ENC(*ppdb) = SQLITE_UTF16NATIVE;

#if SQLITE_VERSION_NUMBER >= 3008008
    //
    // BUGFIX: See ticket [7c151a2f0e22804c].
    //
    SCHEMA_ENC(*ppdb) = SQLITE_UTF16NATIVE;
#endif
  }

  return ret;
}

SQLITE_API const char *WINAPI systemdata_sqlite3_errmsg_interop(systemdata_sqlite3 *db, int *plen)
{
  const char *pval = systemdata_sqlite3_errmsg(db);
  if (plen) *plen = pval ? strlen(pval) : 0;
  return pval;
}

SQLITE_API int WINAPI systemdata_sqlite3_changes_interop(systemdata_sqlite3 *db)
{
  int result;

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_CHANGES)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_changes_interop(): calling systemdata_sqlite3_changes(%p)...\n", db);
#endif

#ifndef NDEBUG
  if (!db)
      systemdata_sqlite3InteropBreak("null database handle for systemdata_sqlite3_changes()");
#endif

  result = systemdata_sqlite3_changes(db);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_CHANGES)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_changes_interop(): systemdata_sqlite3_changes(%p) returned %d.\n", db, result);
#endif

  return result;
}

SQLITE_API int WINAPI systemdata_sqlite3_prepare_interop(systemdata_sqlite3 *db, const char *sql, int nbytes, systemdata_sqlite3_stmt **ppstmt, const char **pztail, int *plen)
{
  int n;

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_PREPARE)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_prepare_interop(): calling systemdata_sqlite3_prepare(%p, \"%s\", %d, %p)...\n", db, sql, nbytes, ppstmt);
#endif

#if SQLITE_VERSION_NUMBER >= 3003009
  n = systemdata_sqlite3_prepare_v2(db, sql, nbytes, ppstmt, pztail);
#else
  n = systemdata_sqlite3_prepare(db, sql, nbytes, ppstmt, pztail);
#endif

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_PREPARE)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_prepare_interop(): systemdata_sqlite3_prepare(%p, \"%s\", %d, %p) returned %d.\n", db, sql, nbytes, ppstmt, n);
#endif

  if (plen) *plen = (pztail && *pztail) ? strlen(*pztail) : 0;

  return n;
}

SQLITE_API int WINAPI systemdata_sqlite3_prepare16_interop(systemdata_sqlite3 *db, const void *sql, int nchars, systemdata_sqlite3_stmt **ppstmt, const void **pztail, int *plen)
{
  int n;

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_PREPARE16)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_prepare_interop(): calling systemdata_sqlite3_prepare16(%p, \"%s\", %d, %p)...\n", db, sql, nchars, ppstmt);
#endif

#if SQLITE_VERSION_NUMBER >= 3003009
  n = systemdata_sqlite3_prepare16_v2(db, sql, nchars * sizeof(wchar_t), ppstmt, pztail);
#else
  n = systemdata_sqlite3_prepare16(db, sql, nchars * sizeof(wchar_t), ppstmt, pztail);
#endif

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_PREPARE16)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_prepare_interop(): systemdata_sqlite3_prepare16(%p, \"%s\", %d, %p) returned %d.\n", db, sql, nchars, ppstmt, n);
#endif

  if (plen) *plen = (pztail && *pztail) ? wcslen((wchar_t *)*pztail) * sizeof(wchar_t) : 0;

  return n;
}

#if defined(INTEROP_VIRTUAL_TABLE) && SQLITE_VERSION_NUMBER >= 3004001
#ifdef _WIN32
__declspec(dllexport)
#endif
void *systemdata_sqlite3_create_disposable_module(
  systemdata_sqlite3 *db,
  const char *zName,
  const systemdata_sqlite3_module *p,
  void *pClientData,
  void(*xDestroy)(void*)
); /* defined in "src/ext/vtshim.c" (included below) */

SQLITE_API void *WINAPI systemdata_sqlite3_create_disposable_module_interop(
  systemdata_sqlite3 *db,
  const char *zName,
  systemdata_sqlite3_module *pModule,
  int iVersion,
  int (*xCreate)(systemdata_sqlite3*, void *, int, const char *const*, systemdata_sqlite3_vtab **, char**),
  int (*xConnect)(systemdata_sqlite3*, void *, int, const char *const*, systemdata_sqlite3_vtab **, char**),
  int (*xBestIndex)(systemdata_sqlite3_vtab *, systemdata_sqlite3_index_info*),
  int (*xDisconnect)(systemdata_sqlite3_vtab *),
  int (*xDestroy)(systemdata_sqlite3_vtab *),
  int (*xOpen)(systemdata_sqlite3_vtab *, systemdata_sqlite3_vtab_cursor **),
  int (*xClose)(systemdata_sqlite3_vtab_cursor*),
  int (*xFilter)(systemdata_sqlite3_vtab_cursor*, int, const char *, int, systemdata_sqlite3_value **),
  int (*xNext)(systemdata_sqlite3_vtab_cursor*),
  int (*xEof)(systemdata_sqlite3_vtab_cursor*),
  int (*xColumn)(systemdata_sqlite3_vtab_cursor*, systemdata_sqlite3_context*, int),
  int (*xRowid)(systemdata_sqlite3_vtab_cursor*, systemdata_sqlite3_int64 *),
  int (*xUpdate)(systemdata_sqlite3_vtab *, int, systemdata_sqlite3_value **, systemdata_sqlite3_int64 *),
  int (*xBegin)(systemdata_sqlite3_vtab *),
  int (*xSync)(systemdata_sqlite3_vtab *),
  int (*xCommit)(systemdata_sqlite3_vtab *),
  int (*xRollback)(systemdata_sqlite3_vtab *),
  int (*xFindFunction)(systemdata_sqlite3_vtab *, int, const char *, void (**pxFunc)(systemdata_sqlite3_context*, int, systemdata_sqlite3_value**), void **ppArg),
  int (*xRename)(systemdata_sqlite3_vtab *, const char *),
  int (*xSavepoint)(systemdata_sqlite3_vtab *, int),
  int (*xRelease)(systemdata_sqlite3_vtab *, int),
  int (*xRollbackTo)(systemdata_sqlite3_vtab *, int),
  void *pClientData,
  void(*xDestroyModule)(void*)
){
  if (!pModule) return 0;
  memset(pModule, 0, sizeof(*pModule));
  pModule->iVersion = iVersion;
  pModule->xCreate = xCreate;
  pModule->xConnect = xConnect;
  pModule->xBestIndex = xBestIndex;
  pModule->xDisconnect = xDisconnect;
  pModule->xDestroy = xDestroy;
  pModule->xOpen = xOpen;
  pModule->xClose = xClose;
  pModule->xFilter = xFilter;
  pModule->xNext = xNext;
  pModule->xEof = xEof;
  pModule->xColumn = xColumn;
  pModule->xRowid = xRowid;
  pModule->xUpdate = xUpdate;
  pModule->xBegin = xBegin;
  pModule->xSync = xSync;
  pModule->xCommit = xCommit;
  pModule->xRollback = xRollback;
  pModule->xFindFunction = xFindFunction;
  pModule->xRename = xRename;
  pModule->xSavepoint = xSavepoint;
  pModule->xRelease = xRelease;
  pModule->xRollbackTo = xRollbackTo;
  return systemdata_sqlite3_create_disposable_module(db, zName, pModule, pClientData, xDestroyModule);
}
#endif

SQLITE_API int WINAPI systemdata_sqlite3_bind_double_interop(systemdata_sqlite3_stmt *stmt, int iCol, double *val)
{
  if (!val) return SQLITE_ERROR;
  return systemdata_sqlite3_bind_double(stmt,iCol,*val);
}

SQLITE_API int WINAPI systemdata_sqlite3_bind_int64_interop(systemdata_sqlite3_stmt *stmt, int iCol, sqlite_int64 *val)
{
  if (!val) return SQLITE_ERROR;
  return systemdata_sqlite3_bind_int64(stmt,iCol,*val);
}

SQLITE_API const char * WINAPI systemdata_sqlite3_bind_parameter_name_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const char *pval = systemdata_sqlite3_bind_parameter_name(stmt, iCol);
  if (plen) *plen = pval ? strlen(pval) : 0;
  return pval;
}

SQLITE_API const char * WINAPI systemdata_sqlite3_column_name_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const char *pval = systemdata_sqlite3_column_name(stmt, iCol);
  if (plen) *plen = pval ? strlen(pval) : 0;
  return pval;
}

SQLITE_API const void * WINAPI systemdata_sqlite3_column_name16_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const void *pval = systemdata_sqlite3_column_name16(stmt, iCol);
  if (plen) *plen = pval ? wcslen((wchar_t *)pval) * sizeof(wchar_t) : 0;
  return pval;
}

SQLITE_API const char * WINAPI systemdata_sqlite3_column_decltype_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const char *pval = systemdata_sqlite3_column_decltype(stmt, iCol);
  if (plen) *plen = pval ? strlen(pval) : 0;
  return pval;
}

SQLITE_API const void * WINAPI systemdata_sqlite3_column_decltype16_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const void *pval = systemdata_sqlite3_column_decltype16(stmt, iCol);
  if (plen) *plen = pval ? wcslen((wchar_t *)pval) * sizeof(wchar_t) : 0;
  return pval;
}

SQLITE_API void WINAPI systemdata_sqlite3_column_double_interop(systemdata_sqlite3_stmt *stmt, int iCol, double *val)
{
  if (!val) return;
  *val = systemdata_sqlite3_column_double(stmt,iCol);
}

SQLITE_API void WINAPI systemdata_sqlite3_column_int64_interop(systemdata_sqlite3_stmt *stmt, int iCol, sqlite_int64 *val)
{
  if (!val) return;
  *val = systemdata_sqlite3_column_int64(stmt,iCol);
}

SQLITE_API void WINAPI systemdata_sqlite3_last_insert_rowid_interop(systemdata_sqlite3 *db, sqlite_int64 *rowId)
{
  if (!rowId) return;
  *rowId = systemdata_sqlite3_last_insert_rowid(db);
}

SQLITE_API void WINAPI systemdata_sqlite3_memory_used_interop(sqlite_int64 *nBytes)
{
  if (!nBytes) return;
  *nBytes = systemdata_sqlite3_memory_used();
}

SQLITE_API void WINAPI systemdata_sqlite3_memory_highwater_interop(int resetFlag, sqlite_int64 *nBytes)
{
  if (!nBytes) return;
  *nBytes = systemdata_sqlite3_memory_highwater(resetFlag);
}

SQLITE_API const unsigned char * WINAPI systemdata_sqlite3_column_text_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const unsigned char *pval = systemdata_sqlite3_column_text(stmt, iCol);
  if (plen) *plen = systemdata_sqlite3_column_bytes(stmt, iCol);
  return pval;
}

SQLITE_API const void * WINAPI systemdata_sqlite3_column_text16_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const void *pval = systemdata_sqlite3_column_text16(stmt, iCol);
  if (plen) *plen = systemdata_sqlite3_column_bytes16(stmt, iCol);
  return pval;
}

SQLITE_API int WINAPI systemdata_sqlite3_finalize_interop(systemdata_sqlite3_stmt *stmt)
{
  int ret;
#if !defined(INTEROP_LEGACY_CLOSE) && SQLITE_VERSION_NUMBER >= 3007014

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_FINALIZE)
  Vdbe *p = (Vdbe *)stmt;
  systemdata_sqlite3 *db = p ? p->db : 0;
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_finalize_interop(): calling systemdata_sqlite3_finalize(%p, %p)...\n", db, stmt);
#endif

  ret = systemdata_sqlite3_finalize(stmt);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_FINALIZE)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_finalize_interop(): systemdata_sqlite3_finalize(%p, %p) returned %d.\n", db, stmt, ret);
#endif

  return ret;
#else
  Vdbe *p;
  ret = SQLITE_OK;

  p = (Vdbe *)stmt;
  if (p)
  {
    systemdata_sqlite3 *db = p->db;

    if (db != NULL)
      systemdata_sqlite3_mutex_enter(db->mutex);

    if ((p->magic == VDBE_MAGIC_DEAD) && (db == NULL))
    {
      systemdata_sqlite3DbFree_interop(db, p);
    }
    else
    {
      ret = systemdata_sqlite3_finalize(stmt);
    }

    if (db != NULL)
      systemdata_sqlite3_mutex_leave(db->mutex);
  }

  return ret;
#endif
}

SQLITE_API int WINAPI systemdata_sqlite3_backup_finish_interop(systemdata_sqlite3_backup *p)
{
  int ret;

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_BACKUP_FINISH)
  systemdata_sqlite3* pDestDb = p ? p->pDestDb : 0;
  systemdata_sqlite3* pSrcDb = p ? p->pSrcDb : 0;
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_backup_finish_interop(): calling systemdata_sqlite3_backup_finish(%p, %p, %p)...\n", pDestDb, pSrcDb, p);
#endif

  ret = systemdata_sqlite3_backup_finish(p);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_BACKUP_FINISH)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_backup_finish_interop(): systemdata_sqlite3_backup_finish(%p, %p, %p) returned %d.\n", pDestDb, pSrcDb, p, ret);
#endif

  return ret;
}

SQLITE_API int WINAPI systemdata_sqlite3_blob_close_interop(systemdata_sqlite3_blob *p)
{
  int ret;

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_BLOB_CLOSE)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_blob_close_interop(): calling systemdata_sqlite3_blob_close(%p)...\n", p);
#endif

  ret = systemdata_sqlite3_blob_close(p);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_BLOB_CLOSE)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_blob_close_interop(): systemdata_sqlite3_blob_close(%p) returned %d.\n", p, ret);
#endif

  return ret;
}

SQLITE_API int WINAPI systemdata_sqlite3_reset_interop(systemdata_sqlite3_stmt *stmt)
{
  int ret;
#if !defined(INTEROP_LEGACY_CLOSE) && SQLITE_VERSION_NUMBER >= 3007014

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_RESET)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_reset_interop(): calling systemdata_sqlite3_reset(%p)...\n", stmt);
#endif

  ret = systemdata_sqlite3_reset(stmt);

#if defined(INTEROP_DEBUG) && (INTEROP_DEBUG & INTEROP_DEBUG_RESET)
  systemdata_sqlite3InteropDebug("systemdata_sqlite3_reset_interop(): systemdata_sqlite3_reset(%p) returned %d.\n", stmt, ret);
#endif

  return ret;
#else
  if (stmt && ((Vdbe *)stmt)->magic == VDBE_MAGIC_DEAD) return SQLITE_SCHEMA;
  ret = systemdata_sqlite3_reset(stmt);
  return ret;
#endif
}

SQLITE_API int WINAPI systemdata_sqlite3_create_function_interop(systemdata_sqlite3 *psql, const char *zFunctionName, int nArg, int eTextRep, void *pvUser, SQLITEUSERFUNC func, SQLITEUSERFUNC funcstep, SQLITEFUNCFINAL funcfinal, int needCollSeq)
{
  int n;

  if (eTextRep == SQLITE_UTF16)
    eTextRep = SQLITE_UTF16NATIVE;

  n = systemdata_sqlite3_create_function(psql, zFunctionName, nArg, eTextRep, pvUser, func, funcstep, funcfinal);
  if (n == SQLITE_OK)
  {
    if (needCollSeq)
    {
      FuncDef *pFunc = systemdata_sqlite3FindFunction(
          psql, zFunctionName,
#if SQLITE_VERSION_NUMBER < 3012000
          strlen(zFunctionName),
#endif
          nArg, eTextRep, 0);
      if( pFunc )
      {
#if SQLITE_VERSION_NUMBER >= 3008001
        pFunc->funcFlags |= SQLITE_FUNC_NEEDCOLL;
#else
        pFunc->flags |= SQLITE_FUNC_NEEDCOLL;
#endif
      }
    }
  }

  return n;
}

SQLITE_API void WINAPI systemdata_sqlite3_value_double_interop(systemdata_sqlite3_value *pval, double *val)
{
  if (!val) return;
  *val = systemdata_sqlite3_value_double(pval);
}

SQLITE_API void WINAPI systemdata_sqlite3_value_int64_interop(systemdata_sqlite3_value *pval, sqlite_int64 *val)
{
  if (!val) return;
  *val = systemdata_sqlite3_value_int64(pval);
}

SQLITE_API const unsigned char * WINAPI systemdata_sqlite3_value_text_interop(systemdata_sqlite3_value *val, int *plen)
{
  const unsigned char *pval = systemdata_sqlite3_value_text(val);
  if (plen) *plen = systemdata_sqlite3_value_bytes(val);
  return pval;
}

SQLITE_API const void * WINAPI systemdata_sqlite3_value_text16_interop(systemdata_sqlite3_value *val, int *plen)
{
  const void *pval = systemdata_sqlite3_value_text16(val);
  if (plen) *plen = systemdata_sqlite3_value_bytes16(val);
  return pval;
}

SQLITE_API void WINAPI systemdata_sqlite3_result_double_interop(systemdata_sqlite3_context *pctx, double *val)
{
  if (!val) return;
  systemdata_sqlite3_result_double(pctx, *val);
}

SQLITE_API void WINAPI systemdata_sqlite3_result_int64_interop(systemdata_sqlite3_context *pctx, sqlite_int64 *val)
{
  if (!val) return;
  systemdata_sqlite3_result_int64(pctx, *val);
}

SQLITE_API int WINAPI systemdata_sqlite3_context_collcompare_interop(systemdata_sqlite3_context *ctx, const void *p1, int p1len, const void *p2, int p2len)
{
#if SQLITE_VERSION_NUMBER >= 3008007
  CollSeq *pColl = ctx ? systemdata_sqlite3GetFuncCollSeq(ctx) : 0;
#else
  CollSeq *pColl = ctx ? ctx->pColl : 0;
#endif
  if (!ctx || !ctx->pFunc) return 4; /* ERROR */
  if (!pColl || !pColl->xCmp) return 3; /* ERROR */
#if SQLITE_VERSION_NUMBER >= 3008001
  if ((ctx->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL) == 0) return 2; /* ERROR */
#else
  if ((ctx->pFunc->flags & SQLITE_FUNC_NEEDCOLL) == 0) return 2; /* ERROR */
#endif
  return pColl->xCmp(pColl->pUser, p1len, p1, p2len, p2);
}

SQLITE_API const char * WINAPI systemdata_sqlite3_context_collseq_interop(systemdata_sqlite3_context *ctx, int *ptype, int *enc, int *plen)
{
#if SQLITE_VERSION_NUMBER >= 3008007
  CollSeq *pColl = ctx ? systemdata_sqlite3GetFuncCollSeq(ctx) : 0;
#else
  CollSeq *pColl = ctx ? ctx->pColl : 0;
#endif
  if (ptype) *ptype = 0;
  if (plen) *plen = 0;
  if (enc) *enc = 0;

  if (!ctx || !ctx->pFunc) return NULL;
#if SQLITE_VERSION_NUMBER >= 3008001
  if ((ctx->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL) == 0) return NULL;
#else
  if ((ctx->pFunc->flags & SQLITE_FUNC_NEEDCOLL) == 0) return NULL;
#endif

  if (pColl)
  {
    if (enc) *enc = pColl->enc;
#if SQLITE_VERSION_NUMBER < 3007010
    if (ptype) *ptype = pColl->type;
#endif
    if (plen) *plen = pColl->zName ? strlen(pColl->zName) : 0;

    return pColl->zName;
  }
  return NULL;
}

SQLITE_API const char * WINAPI systemdata_sqlite3_column_database_name_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const char *pval = systemdata_sqlite3_column_database_name(stmt, iCol);
  if (plen) *plen = pval ? strlen(pval) : 0;
  return pval;
}

SQLITE_API const void * WINAPI systemdata_sqlite3_column_database_name16_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const void *pval = systemdata_sqlite3_column_database_name16(stmt, iCol);
  if (plen) *plen = pval ? wcslen((wchar_t *)pval) * sizeof(wchar_t) : 0;
  return pval;
}

SQLITE_API const char * WINAPI systemdata_sqlite3_column_table_name_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const char *pval = systemdata_sqlite3_column_table_name(stmt, iCol);
  if (plen) *plen = pval ? strlen(pval) : 0;
  return pval;
}

SQLITE_API const void * WINAPI systemdata_sqlite3_column_table_name16_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const void *pval = systemdata_sqlite3_column_table_name16(stmt, iCol);
  if (plen) *plen = pval ? wcslen((wchar_t *)pval) * sizeof(wchar_t) : 0;
  return pval;
}

SQLITE_API const char * WINAPI systemdata_sqlite3_column_origin_name_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const char *pval = systemdata_sqlite3_column_origin_name(stmt, iCol);
  if (plen) *plen = pval ? strlen(pval) : 0;
  return pval;
}

SQLITE_API const void * WINAPI systemdata_sqlite3_column_origin_name16_interop(systemdata_sqlite3_stmt *stmt, int iCol, int *plen)
{
  const void *pval = systemdata_sqlite3_column_origin_name16(stmt, iCol);
  if (plen) *plen = pval ? wcslen((wchar_t *)pval) * sizeof(wchar_t) : 0;
  return pval;
}

SQLITE_API int WINAPI systemdata_sqlite3_table_column_metadata_interop(systemdata_sqlite3 *db, const char *zDbName, const char *zTableName, const char *zColumnName, char const **pzDataType, char const **pzCollSeq, int *pNotNull, int *pPrimaryKey, int *pAutoinc, int *pdtLen, int *pcsLen)
{
  int n;

  n = systemdata_sqlite3_table_column_metadata(db, zDbName, zTableName, zColumnName, pzDataType, pzCollSeq, pNotNull, pPrimaryKey, pAutoinc);

  if (pdtLen) *pdtLen = (pzDataType && *pzDataType) ? strlen(*pzDataType) : 0;
  if (pcsLen) *pcsLen = (pzCollSeq && *pzCollSeq) ? strlen(*pzCollSeq) : 0;

  return n;
}

SQLITE_API int WINAPI systemdata_sqlite3_index_column_info_interop(systemdata_sqlite3 *db, const char *zDb, const char *zIndexName, const char *zColumnName, int *sortOrder, int *onError, const char **pzColl, int *plen)
{
  Index *pIdx;
  Table *pTab;
  int n;

  if (!db) return SQLITE_ERROR;
  systemdata_sqlite3_mutex_enter(db->mutex);
  systemdata_sqlite3BtreeEnterAll(db);

  pIdx = systemdata_sqlite3FindIndex(db, zIndexName, zDb);

  systemdata_sqlite3BtreeLeaveAll(db);
  systemdata_sqlite3_mutex_leave(db->mutex);

  if (!pIdx) return SQLITE_ERROR;

  pTab = pIdx->pTable;
  for (n = 0; n < pIdx->nColumn; n++)
  {
    int cnum = pIdx->aiColumn[n];
    if (systemdata_sqlite3StrICmp(pTab->aCol[cnum].zName, zColumnName) == 0)
    {
      if ( sortOrder ) *sortOrder = pIdx->aSortOrder[n];
      if ( pzColl ) *pzColl = pIdx->azColl[n];
      if ( plen ) *plen = strlen(*pzColl);
      if ( onError ) *onError = pIdx->onError;

      return SQLITE_OK;
    }
  }
  return SQLITE_ERROR;
}

SQLITE_API int WINAPI systemdata_sqlite3_table_cursor_interop(systemdata_sqlite3_stmt *pstmt, int iDb, Pgno tableRootPage)
{
  Vdbe *p = (Vdbe *)pstmt;
  systemdata_sqlite3 *db = (p == NULL) ? NULL : p->db;
  int n;
  int ret = -1; /* NOT FOUND */

  if (!p || !db) return ret;
  systemdata_sqlite3_mutex_enter(db->mutex);
  for (n = 0; n < p->nCursor && p->apCsr[n] != NULL; n++)
  {
    if (p->apCsr[n]->isTable == 0) continue;
    if (p->apCsr[n]->iDb != iDb) continue;
#if SQLITE_VERSION_NUMBER >= 3010000
    if (p->apCsr[n]->uc.pCursor->pgnoRoot == tableRootPage)
#else
    if (p->apCsr[n]->pCursor->pgnoRoot == tableRootPage)
#endif
    {
      ret = n;
      break;
    }
  }
  systemdata_sqlite3_mutex_leave(db->mutex);

  return ret;
}

SQLITE_API int WINAPI systemdata_sqlite3_cursor_rowid_interop(systemdata_sqlite3_stmt *pstmt, int cursor, sqlite_int64 *prowid)
{
  Vdbe *p = (Vdbe *)pstmt;
  systemdata_sqlite3 *db = (p == NULL) ? NULL : p->db;
  VdbeCursor *pC;
#if SQLITE_VERSION_NUMBER >= 3011000
  int p2 = 0;
#endif
  int ret = SQLITE_OK;

  if (!p || !db) return SQLITE_ERROR;
  systemdata_sqlite3_mutex_enter(db->mutex);
  while (1)
  {
    if (cursor < 0 || cursor >= p->nCursor)
    {
      ret = SQLITE_ERROR;
      break;
    }
    if (p->apCsr[cursor] == NULL)
    {
      ret = SQLITE_ERROR;
      break;
    }

    pC = p->apCsr[cursor];

#if SQLITE_VERSION_NUMBER >= 3011000
    ret = systemdata_sqlite3VdbeCursorMoveto(&pC, &p2);
#else
    ret = systemdata_sqlite3VdbeCursorMoveto(pC);
#endif
    if(ret)
      break;

#if SQLITE_VERSION_NUMBER < 3008007
    if(pC->rowidIsValid)
    {
      if (prowid) *prowid = pC->lastRowid;
    }
    else
#endif
#if SQLITE_VERSION_NUMBER >= 3010000
    if(pC->eCurType != CURTYPE_BTREE)
#else
    if(pC->pseudoTableReg > 0)
#endif
    {
      ret = SQLITE_ERROR;
      break;
    }
#if SQLITE_VERSION_NUMBER >= 3010000
    else if(pC->nullRow || pC->uc.pCursor==0)
#else
    else if(pC->nullRow || pC->pCursor==0)
#endif
    {
      ret = SQLITE_ERROR;
      break;
    }
    else
    {
#if SQLITE_VERSION_NUMBER >= 3010000
      if (pC->uc.pCursor == NULL)
#else
      if (pC->pCursor == NULL)
#endif
      {
        ret = SQLITE_ERROR;
        break;
      }
#if SQLITE_VERSION_NUMBER >= 3014000
      systemdata_sqlite3BtreeEnterCursor(pC->uc.pCursor);
      *prowid = systemdata_sqlite3BtreeIntegerKey(pC->uc.pCursor);
      systemdata_sqlite3BtreeLeaveCursor(pC->uc.pCursor);
#elif SQLITE_VERSION_NUMBER >= 3010000
      systemdata_sqlite3BtreeKeySize(pC->uc.pCursor, prowid);
#else
      systemdata_sqlite3BtreeKeySize(pC->pCursor, prowid);
#endif
      if (prowid) *prowid = *prowid;
    }
    break;
  }
  systemdata_sqlite3_mutex_leave(db->mutex);

  return ret;
}

/*****************************************************************************/

#if defined(SQLITE_CORE)
#undef SQLITE_CORE
#endif

#if defined(INTEROP_VIRTUAL_TABLE) && SQLITE_VERSION_NUMBER >= 3004001
#include "../ext/vtshim.c"
#endif

#if defined(INTEROP_FTS5_EXTENSION)
#include "../ext/fts5.c"
#endif

#if defined(INTEROP_JSON1_EXTENSION)
#include "../ext/json1.c"
#endif

#if defined(INTEROP_PERCENTILE_EXTENSION)
#include "../ext/percentile.c"
#endif

#if defined(INTEROP_REGEXP_EXTENSION)
#include "../ext/regexp.c"
#endif

#if defined(INTEROP_SHA1_EXTENSION)
#include "../ext/sha1.c"
#endif

#if defined(INTEROP_TOTYPE_EXTENSION)
#include "../ext/totype.c"
#endif

/*****************************************************************************/

/*
** The INTEROP_TEST_EXTENSION block must be at the end of this source file
** because it includes the "systemdata_sqlite3ext.h" file, which defines the systemdata_sqlite3
** public API function names to be macros and that would cause the code
** above this point to malfunction.
*/
#if defined(INTEROP_TEST_EXTENSION)
#if !SQLITE_OS_WIN
#include <unistd.h>
#endif

#include "systemdata_sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The interopTest() SQL function returns its first argument or raises an
** error if there are not enough arguments.
*/
SQLITE_PRIVATE void interopTestFunc(
  systemdata_sqlite3_context *context,
  int argc,
  systemdata_sqlite3_value **argv
){
  const unsigned char *z;
  if( argc!=1 ){
    systemdata_sqlite3_result_error(context, "need exactly one argument", -1);
    return;
  }
  z = systemdata_sqlite3_value_text(argv[0]);
  if( z ){
    systemdata_sqlite3_result_text(context, (char*)z, -1, SQLITE_STATIC);
  }else{
    systemdata_sqlite3_result_null(context);
  }
}

/*
** The interopSleep() SQL function waits the specified number of milliseconds
** or raises an error if there are not enough arguments.
*/
SQLITE_PRIVATE void interopSleepFunc(
  systemdata_sqlite3_context *context,
  int argc,
  systemdata_sqlite3_value **argv
){
  int m;
  if( argc!=1 ){
    systemdata_sqlite3_result_error(context, "need exactly one argument", -1);
    return;
  }
  m = systemdata_sqlite3_value_int(argv[0]);
#if SQLITE_OS_WIN
#if SQLITE_OS_WINCE
  Sleep(m);
  systemdata_sqlite3_result_int(context, WAIT_OBJECT_0);
#else
  systemdata_sqlite3_result_int(context, SleepEx(m, TRUE));
#endif
#else
  if( m>0 ){
    systemdata_sqlite3_result_int64(context, sleep((unsigned)m));
  }else{
    systemdata_sqlite3_result_null(context);
  }
#endif
}

/* SQLite invokes this routine once when it loads the extension.
** Create new functions, collating sequences, and virtual table
** modules here.  This is usually the only exported symbol in
** the shared library.
*/
SQLITE_API int interop_test_extension_init(
  systemdata_sqlite3 *db,
  char **pzErrMsg,
  const systemdata_sqlite3_api_routines *pApi
){
  int rc;
  SQLITE_EXTENSION_INIT2(pApi)
  rc = systemdata_sqlite3_create_function(db, "interopTest", -1, SQLITE_ANY, 0,
      interopTestFunc, 0, 0);
  if( rc==SQLITE_OK ){
    rc = systemdata_sqlite3_create_function(db, "interopSleep", 1, SQLITE_ANY, 0,
        interopSleepFunc, 0, 0);
  }
  return rc;
}
#endif /* SQLITE_OS_WIN */

