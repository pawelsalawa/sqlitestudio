/*
** Enable SQLite debug assertions if requested
*/
#ifndef SQLITE_DEBUG
#if defined(SQLITE_ENABLE_DEBUG) && (SQLITE_ENABLE_DEBUG == 1)
#define SQLITE_DEBUG 1
#endif
#endif

/*
** To enable the extension functions define SQLITE_ENABLE_EXTFUNC on compiling this module
** To enable the reading CSV files define SQLITE_ENABLE_CSV on compiling this module
** To enable the SHA3 support define SQLITE_ENABLE_SHA3 on compiling this module
** To enable the CARRAY support define SQLITE_ENABLE_CARRAY on compiling this module
** To enable the FILEIO support define SQLITE_ENABLE_FILEIO on compiling this module
** To enable the SERIES support define SQLITE_ENABLE_SERIES on compiling this module
*/
#if defined(SQLITE_ENABLE_EXTFUNC) || defined(SQLITE_ENABLE_CSV) || defined(SQLITE_ENABLE_SHA3) || defined(SQLITE_ENABLE_CARRAY) || defined(SQLITE_ENABLE_FILEIO) || defined(SQLITE_ENABLE_SERIES)
#define wx_sqlite3_open    wx_sqlite3_open_internal
#define wx_sqlite3_open16  wx_sqlite3_open16_internal
#define wx_sqlite3_open_v2 wx_sqlite3_open_v2_internal
#endif

/*
** Enable the user authentication feature
*/
#ifndef SQLITE_USER_AUTHENTICATION
#define SQLITE_USER_AUTHENTICATION 1
#endif

#include "wxsqlite3.c"
#ifdef SQLITE_USER_AUTHENTICATION
#include "sha2.h"
#include "sha2.c"
#include "userauth.c"
#endif

#if defined(SQLITE_ENABLE_EXTFUNC) || defined(SQLITE_ENABLE_CSV) || defined(SQLITE_ENABLE_SHA3) || defined(SQLITE_ENABLE_CARRAY) || defined(SQLITE_ENABLE_FILEIO) || defined(SQLITE_ENABLE_SERIES)
#undef wx_sqlite3_open
#undef wx_sqlite3_open16
#undef wx_sqlite3_open_v2
#endif

#ifndef SQLITE_OMIT_DISKIO

#ifdef SQLITE_HAS_CODEC

/*
** Get the codec argument for this pager
*/

void* mySqlite3PagerGetCodec(
  Pager *pPager
){
#if (SQLITE_VERSION_NUMBER >= 3006016)
  return wx_sqlite3PagerGetCodec(pPager);
#else
  return (pPager->xCodec) ? pPager->pCodecArg : NULL;
#endif
}

/*
** Set the codec argument for this pager
*/

void mySqlite3PagerSetCodec(
  Pager *pPager,
  void *(*xCodec)(void*,void*,Pgno,int),
  void (*xCodecSizeChng)(void*,int,int),
  void (*xCodecFree)(void*),
  void *pCodec
){
  wx_sqlite3PagerSetCodec(pPager, xCodec, xCodecSizeChng, xCodecFree, pCodec);
}

#include "rijndael.c"
#include "codec.c"
#include "codecext.c"

#endif

#endif

/*
** Extension functions
*/
#ifdef SQLITE_ENABLE_EXTFUNC
#include "extensionfunctions.c"
#endif

/*
** CSV import
*/
#ifdef SQLITE_ENABLE_CSV
#include "csv.c"
#endif

/*
** SHA3
*/
#ifdef SQLITE_ENABLE_SHA3
#include "shathree.c"
#endif

/*
** CARRAY
*/
#ifdef SQLITE_ENABLE_CARRAY
#include "carray.c"
#endif

/*
** FILEIO
*/
#ifdef SQLITE_ENABLE_FILEIO

/* MinGW specifics */
#if (!defined(_WIN32) && !defined(WIN32)) || defined(__MINGW32__)
# include <unistd.h>
# include <dirent.h>
# if defined(__MINGW32__)
#  define DIRENT dirent
#  ifndef S_ISLNK
#   define S_ISLNK(mode) (0)
#  endif
# endif
#endif

#include "test_windirent.c"
#include "fileio.c"
#endif

/*
** SERIES
*/
#ifdef SQLITE_ENABLE_SERIES
#include "series.c"
#endif

#if defined(SQLITE_ENABLE_EXTFUNC) || defined(SQLITE_ENABLE_CSV) || defined(SQLITE_ENABLE_SHA3) || defined(SQLITE_ENABLE_CARRAY) || defined(SQLITE_ENABLE_FILEIO) || defined(SQLITE_ENABLE_SERIES)

static
void registerAllExtensions(wx_sqlite3 *db, char **pzErrMsg, const wx_sqlite3_api_routines *pApi)
{
#ifdef SQLITE_ENABLE_EXTFUNC
    RegisterExtensionFunctions(db);
#endif
#ifdef SQLITE_ENABLE_CSV
    wx_sqlite3_csv_init(db, NULL, NULL);
#endif
#ifdef SQLITE_ENABLE_SHA3
    wx_sqlite3_shathree_init(db, NULL, NULL);
#endif
#ifdef SQLITE_ENABLE_CARRAY
    wx_sqlite3_carray_init(db, NULL, NULL);
#endif
#ifdef SQLITE_ENABLE_FILEIO
    wx_sqlite3_fileio_init(db, NULL, NULL);
#endif
#ifdef SQLITE_ENABLE_SERIES
    wx_sqlite3_series_init(db, NULL, NULL);
#endif
}

SQLITE_API int wx_sqlite3_open(
  const char *filename,   /* Database filename (UTF-8) */
  wx_sqlite3 **ppDb          /* OUT: SQLite db handle */
)
{
  int ret = wx_sqlite3_open_internal(filename, ppDb);
  if (ret == 0)
  {
    registerAllExtensions(*ppDb, NULL, NULL);
  }
  return ret;
}

SQLITE_API int wx_sqlite3_open16(
  const void *filename,   /* Database filename (UTF-16) */
  wx_sqlite3 **ppDb          /* OUT: SQLite db handle */
)
{
  int ret = wx_sqlite3_open16_internal(filename, ppDb);
  if (ret == 0)
  {
    registerAllExtensions(*ppDb, NULL, NULL);
  }
  return ret;
}

SQLITE_API int wx_sqlite3_open_v2(
  const char *filename,   /* Database filename (UTF-8) */
  wx_sqlite3 **ppDb,         /* OUT: SQLite db handle */
  int flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
)
{
  int ret = wx_sqlite3_open_v2_internal(filename, ppDb, flags, zVfs);
  if (ret == 0)
  {
    registerAllExtensions(*ppDb, NULL, NULL);
  }
  return ret;
}

#endif


