/*
** 2006 June 7
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This header file defines the SQLite interface for use by
** shared libraries that want to be imported as extensions into
** an SQLite instance.  Shared libraries that intend to be loaded
** as extensions by SQLite should #include this file instead of 
** systemdata_sqlite3.h.
*/
#ifndef SQLITE3EXT_H
#define SQLITE3EXT_H
#include "systemdata_sqlite3.h"

/*
** The following structure holds pointers to all of the SQLite API
** routines.
**
** WARNING:  In order to maintain backwards compatibility, add new
** interfaces to the end of this structure only.  If you insert new
** interfaces in the middle of this structure, then older different
** versions of SQLite will not be able to load each other's shared
** libraries!
*/
struct systemdata_sqlite3_api_routines {
  void * (*aggregate_context)(systemdata_sqlite3_context*,int nBytes);
  int  (*aggregate_count)(systemdata_sqlite3_context*);
  int  (*bind_blob)(systemdata_sqlite3_stmt*,int,const void*,int n,void(*)(void*));
  int  (*bind_double)(systemdata_sqlite3_stmt*,int,double);
  int  (*bind_int)(systemdata_sqlite3_stmt*,int,int);
  int  (*bind_int64)(systemdata_sqlite3_stmt*,int,sqlite_int64);
  int  (*bind_null)(systemdata_sqlite3_stmt*,int);
  int  (*bind_parameter_count)(systemdata_sqlite3_stmt*);
  int  (*bind_parameter_index)(systemdata_sqlite3_stmt*,const char*zName);
  const char * (*bind_parameter_name)(systemdata_sqlite3_stmt*,int);
  int  (*bind_text)(systemdata_sqlite3_stmt*,int,const char*,int n,void(*)(void*));
  int  (*bind_text16)(systemdata_sqlite3_stmt*,int,const void*,int,void(*)(void*));
  int  (*bind_value)(systemdata_sqlite3_stmt*,int,const systemdata_sqlite3_value*);
  int  (*busy_handler)(systemdata_sqlite3*,int(*)(void*,int),void*);
  int  (*busy_timeout)(systemdata_sqlite3*,int ms);
  int  (*changes)(systemdata_sqlite3*);
  int  (*close)(systemdata_sqlite3*);
  int  (*collation_needed)(systemdata_sqlite3*,void*,void(*)(void*,systemdata_sqlite3*,
                           int eTextRep,const char*));
  int  (*collation_needed16)(systemdata_sqlite3*,void*,void(*)(void*,systemdata_sqlite3*,
                             int eTextRep,const void*));
  const void * (*column_blob)(systemdata_sqlite3_stmt*,int iCol);
  int  (*column_bytes)(systemdata_sqlite3_stmt*,int iCol);
  int  (*column_bytes16)(systemdata_sqlite3_stmt*,int iCol);
  int  (*column_count)(systemdata_sqlite3_stmt*pStmt);
  const char * (*column_database_name)(systemdata_sqlite3_stmt*,int);
  const void * (*column_database_name16)(systemdata_sqlite3_stmt*,int);
  const char * (*column_decltype)(systemdata_sqlite3_stmt*,int i);
  const void * (*column_decltype16)(systemdata_sqlite3_stmt*,int);
  double  (*column_double)(systemdata_sqlite3_stmt*,int iCol);
  int  (*column_int)(systemdata_sqlite3_stmt*,int iCol);
  sqlite_int64  (*column_int64)(systemdata_sqlite3_stmt*,int iCol);
  const char * (*column_name)(systemdata_sqlite3_stmt*,int);
  const void * (*column_name16)(systemdata_sqlite3_stmt*,int);
  const char * (*column_origin_name)(systemdata_sqlite3_stmt*,int);
  const void * (*column_origin_name16)(systemdata_sqlite3_stmt*,int);
  const char * (*column_table_name)(systemdata_sqlite3_stmt*,int);
  const void * (*column_table_name16)(systemdata_sqlite3_stmt*,int);
  const unsigned char * (*column_text)(systemdata_sqlite3_stmt*,int iCol);
  const void * (*column_text16)(systemdata_sqlite3_stmt*,int iCol);
  int  (*column_type)(systemdata_sqlite3_stmt*,int iCol);
  systemdata_sqlite3_value* (*column_value)(systemdata_sqlite3_stmt*,int iCol);
  void * (*commit_hook)(systemdata_sqlite3*,int(*)(void*),void*);
  int  (*complete)(const char*sql);
  int  (*complete16)(const void*sql);
  int  (*create_collation)(systemdata_sqlite3*,const char*,int,void*,
                           int(*)(void*,int,const void*,int,const void*));
  int  (*create_collation16)(systemdata_sqlite3*,const void*,int,void*,
                             int(*)(void*,int,const void*,int,const void*));
  int  (*create_function)(systemdata_sqlite3*,const char*,int,int,void*,
                          void (*xFunc)(systemdata_sqlite3_context*,int,systemdata_sqlite3_value**),
                          void (*xStep)(systemdata_sqlite3_context*,int,systemdata_sqlite3_value**),
                          void (*xFinal)(systemdata_sqlite3_context*));
  int  (*create_function16)(systemdata_sqlite3*,const void*,int,int,void*,
                            void (*xFunc)(systemdata_sqlite3_context*,int,systemdata_sqlite3_value**),
                            void (*xStep)(systemdata_sqlite3_context*,int,systemdata_sqlite3_value**),
                            void (*xFinal)(systemdata_sqlite3_context*));
  int (*create_module)(systemdata_sqlite3*,const char*,const systemdata_sqlite3_module*,void*);
  int  (*data_count)(systemdata_sqlite3_stmt*pStmt);
  systemdata_sqlite3 * (*db_handle)(systemdata_sqlite3_stmt*);
  int (*declare_vtab)(systemdata_sqlite3*,const char*);
  int  (*enable_shared_cache)(int);
  int  (*errcode)(systemdata_sqlite3*db);
  const char * (*errmsg)(systemdata_sqlite3*);
  const void * (*errmsg16)(systemdata_sqlite3*);
  int  (*exec)(systemdata_sqlite3*,const char*,systemdata_sqlite3_callback,void*,char**);
  int  (*expired)(systemdata_sqlite3_stmt*);
  int  (*finalize)(systemdata_sqlite3_stmt*pStmt);
  void  (*free)(void*);
  void  (*free_table)(char**result);
  int  (*get_autocommit)(systemdata_sqlite3*);
  void * (*get_auxdata)(systemdata_sqlite3_context*,int);
  int  (*get_table)(systemdata_sqlite3*,const char*,char***,int*,int*,char**);
  int  (*global_recover)(void);
  void  (*interruptx)(systemdata_sqlite3*);
  sqlite_int64  (*last_insert_rowid)(systemdata_sqlite3*);
  const char * (*libversion)(void);
  int  (*libversion_number)(void);
  void *(*malloc)(int);
  char * (*mprintf)(const char*,...);
  int  (*open)(const char*,systemdata_sqlite3**);
  int  (*open16)(const void*,systemdata_sqlite3**);
  int  (*prepare)(systemdata_sqlite3*,const char*,int,systemdata_sqlite3_stmt**,const char**);
  int  (*prepare16)(systemdata_sqlite3*,const void*,int,systemdata_sqlite3_stmt**,const void**);
  void * (*profile)(systemdata_sqlite3*,void(*)(void*,const char*,sqlite_uint64),void*);
  void  (*progress_handler)(systemdata_sqlite3*,int,int(*)(void*),void*);
  void *(*realloc)(void*,int);
  int  (*reset)(systemdata_sqlite3_stmt*pStmt);
  void  (*result_blob)(systemdata_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_double)(systemdata_sqlite3_context*,double);
  void  (*result_error)(systemdata_sqlite3_context*,const char*,int);
  void  (*result_error16)(systemdata_sqlite3_context*,const void*,int);
  void  (*result_int)(systemdata_sqlite3_context*,int);
  void  (*result_int64)(systemdata_sqlite3_context*,sqlite_int64);
  void  (*result_null)(systemdata_sqlite3_context*);
  void  (*result_text)(systemdata_sqlite3_context*,const char*,int,void(*)(void*));
  void  (*result_text16)(systemdata_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_text16be)(systemdata_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_text16le)(systemdata_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_value)(systemdata_sqlite3_context*,systemdata_sqlite3_value*);
  void * (*rollback_hook)(systemdata_sqlite3*,void(*)(void*),void*);
  int  (*set_authorizer)(systemdata_sqlite3*,int(*)(void*,int,const char*,const char*,
                         const char*,const char*),void*);
  void  (*set_auxdata)(systemdata_sqlite3_context*,int,void*,void (*)(void*));
  char * (*snprintf)(int,char*,const char*,...);
  int  (*step)(systemdata_sqlite3_stmt*);
  int  (*table_column_metadata)(systemdata_sqlite3*,const char*,const char*,const char*,
                                char const**,char const**,int*,int*,int*);
  void  (*thread_cleanup)(void);
  int  (*total_changes)(systemdata_sqlite3*);
  void * (*trace)(systemdata_sqlite3*,void(*xTrace)(void*,const char*),void*);
  int  (*transfer_bindings)(systemdata_sqlite3_stmt*,systemdata_sqlite3_stmt*);
  void * (*update_hook)(systemdata_sqlite3*,void(*)(void*,int ,char const*,char const*,
                                         sqlite_int64),void*);
  void * (*user_data)(systemdata_sqlite3_context*);
  const void * (*value_blob)(systemdata_sqlite3_value*);
  int  (*value_bytes)(systemdata_sqlite3_value*);
  int  (*value_bytes16)(systemdata_sqlite3_value*);
  double  (*value_double)(systemdata_sqlite3_value*);
  int  (*value_int)(systemdata_sqlite3_value*);
  sqlite_int64  (*value_int64)(systemdata_sqlite3_value*);
  int  (*value_numeric_type)(systemdata_sqlite3_value*);
  const unsigned char * (*value_text)(systemdata_sqlite3_value*);
  const void * (*value_text16)(systemdata_sqlite3_value*);
  const void * (*value_text16be)(systemdata_sqlite3_value*);
  const void * (*value_text16le)(systemdata_sqlite3_value*);
  int  (*value_type)(systemdata_sqlite3_value*);
  char *(*vmprintf)(const char*,va_list);
  /* Added ??? */
  int (*overload_function)(systemdata_sqlite3*, const char *zFuncName, int nArg);
  /* Added by 3.3.13 */
  int (*prepare_v2)(systemdata_sqlite3*,const char*,int,systemdata_sqlite3_stmt**,const char**);
  int (*prepare16_v2)(systemdata_sqlite3*,const void*,int,systemdata_sqlite3_stmt**,const void**);
  int (*clear_bindings)(systemdata_sqlite3_stmt*);
  /* Added by 3.4.1 */
  int (*create_module_v2)(systemdata_sqlite3*,const char*,const systemdata_sqlite3_module*,void*,
                          void (*xDestroy)(void *));
  /* Added by 3.5.0 */
  int (*bind_zeroblob)(systemdata_sqlite3_stmt*,int,int);
  int (*blob_bytes)(systemdata_sqlite3_blob*);
  int (*blob_close)(systemdata_sqlite3_blob*);
  int (*blob_open)(systemdata_sqlite3*,const char*,const char*,const char*,systemdata_sqlite3_int64,
                   int,systemdata_sqlite3_blob**);
  int (*blob_read)(systemdata_sqlite3_blob*,void*,int,int);
  int (*blob_write)(systemdata_sqlite3_blob*,const void*,int,int);
  int (*create_collation_v2)(systemdata_sqlite3*,const char*,int,void*,
                             int(*)(void*,int,const void*,int,const void*),
                             void(*)(void*));
  int (*file_control)(systemdata_sqlite3*,const char*,int,void*);
  systemdata_sqlite3_int64 (*memory_highwater)(int);
  systemdata_sqlite3_int64 (*memory_used)(void);
  systemdata_sqlite3_mutex *(*mutex_alloc)(int);
  void (*mutex_enter)(systemdata_sqlite3_mutex*);
  void (*mutex_free)(systemdata_sqlite3_mutex*);
  void (*mutex_leave)(systemdata_sqlite3_mutex*);
  int (*mutex_try)(systemdata_sqlite3_mutex*);
  int (*open_v2)(const char*,systemdata_sqlite3**,int,const char*);
  int (*release_memory)(int);
  void (*result_error_nomem)(systemdata_sqlite3_context*);
  void (*result_error_toobig)(systemdata_sqlite3_context*);
  int (*sleep)(int);
  void (*soft_heap_limit)(int);
  systemdata_sqlite3_vfs *(*vfs_find)(const char*);
  int (*vfs_register)(systemdata_sqlite3_vfs*,int);
  int (*vfs_unregister)(systemdata_sqlite3_vfs*);
  int (*xthreadsafe)(void);
  void (*result_zeroblob)(systemdata_sqlite3_context*,int);
  void (*result_error_code)(systemdata_sqlite3_context*,int);
  int (*test_control)(int, ...);
  void (*randomness)(int,void*);
  systemdata_sqlite3 *(*context_db_handle)(systemdata_sqlite3_context*);
  int (*extended_result_codes)(systemdata_sqlite3*,int);
  int (*limit)(systemdata_sqlite3*,int,int);
  systemdata_sqlite3_stmt *(*next_stmt)(systemdata_sqlite3*,systemdata_sqlite3_stmt*);
  const char *(*sql)(systemdata_sqlite3_stmt*);
  int (*status)(int,int*,int*,int);
  int (*backup_finish)(systemdata_sqlite3_backup*);
  systemdata_sqlite3_backup *(*backup_init)(systemdata_sqlite3*,const char*,systemdata_sqlite3*,const char*);
  int (*backup_pagecount)(systemdata_sqlite3_backup*);
  int (*backup_remaining)(systemdata_sqlite3_backup*);
  int (*backup_step)(systemdata_sqlite3_backup*,int);
  const char *(*compileoption_get)(int);
  int (*compileoption_used)(const char*);
  int (*create_function_v2)(systemdata_sqlite3*,const char*,int,int,void*,
                            void (*xFunc)(systemdata_sqlite3_context*,int,systemdata_sqlite3_value**),
                            void (*xStep)(systemdata_sqlite3_context*,int,systemdata_sqlite3_value**),
                            void (*xFinal)(systemdata_sqlite3_context*),
                            void(*xDestroy)(void*));
  int (*db_config)(systemdata_sqlite3*,int,...);
  systemdata_sqlite3_mutex *(*db_mutex)(systemdata_sqlite3*);
  int (*db_status)(systemdata_sqlite3*,int,int*,int*,int);
  int (*extended_errcode)(systemdata_sqlite3*);
  void (*log)(int,const char*,...);
  systemdata_sqlite3_int64 (*soft_heap_limit64)(systemdata_sqlite3_int64);
  const char *(*sourceid)(void);
  int (*stmt_status)(systemdata_sqlite3_stmt*,int,int);
  int (*strnicmp)(const char*,const char*,int);
  int (*unlock_notify)(systemdata_sqlite3*,void(*)(void**,int),void*);
  int (*wal_autocheckpoint)(systemdata_sqlite3*,int);
  int (*wal_checkpoint)(systemdata_sqlite3*,const char*);
  void *(*wal_hook)(systemdata_sqlite3*,int(*)(void*,systemdata_sqlite3*,const char*,int),void*);
  int (*blob_reopen)(systemdata_sqlite3_blob*,systemdata_sqlite3_int64);
  int (*vtab_config)(systemdata_sqlite3*,int op,...);
  int (*vtab_on_conflict)(systemdata_sqlite3*);
  /* Version 3.7.16 and later */
  int (*close_v2)(systemdata_sqlite3*);
  const char *(*db_filename)(systemdata_sqlite3*,const char*);
  int (*db_readonly)(systemdata_sqlite3*,const char*);
  int (*db_release_memory)(systemdata_sqlite3*);
  const char *(*errstr)(int);
  int (*stmt_busy)(systemdata_sqlite3_stmt*);
  int (*stmt_readonly)(systemdata_sqlite3_stmt*);
  int (*stricmp)(const char*,const char*);
  int (*uri_boolean)(const char*,const char*,int);
  systemdata_sqlite3_int64 (*uri_int64)(const char*,const char*,systemdata_sqlite3_int64);
  const char *(*uri_parameter)(const char*,const char*);
  char *(*vsnprintf)(int,char*,const char*,va_list);
  int (*wal_checkpoint_v2)(systemdata_sqlite3*,const char*,int,int*,int*);
  /* Version 3.8.7 and later */
  int (*auto_extension)(void(*)(void));
  int (*bind_blob64)(systemdata_sqlite3_stmt*,int,const void*,systemdata_sqlite3_uint64,
                     void(*)(void*));
  int (*bind_text64)(systemdata_sqlite3_stmt*,int,const char*,systemdata_sqlite3_uint64,
                      void(*)(void*),unsigned char);
  int (*cancel_auto_extension)(void(*)(void));
  int (*load_extension)(systemdata_sqlite3*,const char*,const char*,char**);
  void *(*malloc64)(systemdata_sqlite3_uint64);
  systemdata_sqlite3_uint64 (*msize)(void*);
  void *(*realloc64)(void*,systemdata_sqlite3_uint64);
  void (*reset_auto_extension)(void);
  void (*result_blob64)(systemdata_sqlite3_context*,const void*,systemdata_sqlite3_uint64,
                        void(*)(void*));
  void (*result_text64)(systemdata_sqlite3_context*,const char*,systemdata_sqlite3_uint64,
                         void(*)(void*), unsigned char);
  int (*strglob)(const char*,const char*);
  /* Version 3.8.11 and later */
  systemdata_sqlite3_value *(*value_dup)(const systemdata_sqlite3_value*);
  void (*value_free)(systemdata_sqlite3_value*);
  int (*result_zeroblob64)(systemdata_sqlite3_context*,systemdata_sqlite3_uint64);
  int (*bind_zeroblob64)(systemdata_sqlite3_stmt*, int, systemdata_sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned int (*value_subtype)(systemdata_sqlite3_value*);
  void (*result_subtype)(systemdata_sqlite3_context*,unsigned int);
  /* Version 3.10.0 and later */
  int (*status64)(int,systemdata_sqlite3_int64*,systemdata_sqlite3_int64*,int);
  int (*strlike)(const char*,const char*,unsigned int);
  int (*db_cacheflush)(systemdata_sqlite3*);
  /* Version 3.12.0 and later */
  int (*system_errno)(systemdata_sqlite3*);
  /* Version 3.14.0 and later */
  int (*trace_v2)(systemdata_sqlite3*,unsigned,int(*)(unsigned,void*,void*,void*),void*);
  char *(*expanded_sql)(systemdata_sqlite3_stmt*);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
typedef int (*systemdata_sqlite3_loadext_entry)(
  systemdata_sqlite3 *db,                       /* Handle to the database. */
  char **pzErrMsg,                   /* Used to set error string on failure. */
  const systemdata_sqlite3_api_routines *pThunk /* Extension API function pointers. */
);

/*
** The following macros redefine the API routines so that they are
** redirected through the global systemdata_sqlite3_api structure.
**
** This header file is also used by the loadext.c source file
** (part of the main SQLite library - not an extension) so that
** it can get access to the systemdata_sqlite3_api_routines structure
** definition.  But the main library does not want to redefine
** the API.  So the redefinition macros are only valid if the
** SQLITE_CORE macros is undefined.
*/
#if !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define systemdata_sqlite3_aggregate_context      systemdata_sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define systemdata_sqlite3_aggregate_count        systemdata_sqlite3_api->aggregate_count
#endif
#define systemdata_sqlite3_bind_blob              systemdata_sqlite3_api->bind_blob
#define systemdata_sqlite3_bind_double            systemdata_sqlite3_api->bind_double
#define systemdata_sqlite3_bind_int               systemdata_sqlite3_api->bind_int
#define systemdata_sqlite3_bind_int64             systemdata_sqlite3_api->bind_int64
#define systemdata_sqlite3_bind_null              systemdata_sqlite3_api->bind_null
#define systemdata_sqlite3_bind_parameter_count   systemdata_sqlite3_api->bind_parameter_count
#define systemdata_sqlite3_bind_parameter_index   systemdata_sqlite3_api->bind_parameter_index
#define systemdata_sqlite3_bind_parameter_name    systemdata_sqlite3_api->bind_parameter_name
#define systemdata_sqlite3_bind_text              systemdata_sqlite3_api->bind_text
#define systemdata_sqlite3_bind_text16            systemdata_sqlite3_api->bind_text16
#define systemdata_sqlite3_bind_value             systemdata_sqlite3_api->bind_value
#define systemdata_sqlite3_busy_handler           systemdata_sqlite3_api->busy_handler
#define systemdata_sqlite3_busy_timeout           systemdata_sqlite3_api->busy_timeout
#define systemdata_sqlite3_changes                systemdata_sqlite3_api->changes
#define systemdata_sqlite3_close                  systemdata_sqlite3_api->close
#define systemdata_sqlite3_collation_needed       systemdata_sqlite3_api->collation_needed
#define systemdata_sqlite3_collation_needed16     systemdata_sqlite3_api->collation_needed16
#define systemdata_sqlite3_column_blob            systemdata_sqlite3_api->column_blob
#define systemdata_sqlite3_column_bytes           systemdata_sqlite3_api->column_bytes
#define systemdata_sqlite3_column_bytes16         systemdata_sqlite3_api->column_bytes16
#define systemdata_sqlite3_column_count           systemdata_sqlite3_api->column_count
#define systemdata_sqlite3_column_database_name   systemdata_sqlite3_api->column_database_name
#define systemdata_sqlite3_column_database_name16 systemdata_sqlite3_api->column_database_name16
#define systemdata_sqlite3_column_decltype        systemdata_sqlite3_api->column_decltype
#define systemdata_sqlite3_column_decltype16      systemdata_sqlite3_api->column_decltype16
#define systemdata_sqlite3_column_double          systemdata_sqlite3_api->column_double
#define systemdata_sqlite3_column_int             systemdata_sqlite3_api->column_int
#define systemdata_sqlite3_column_int64           systemdata_sqlite3_api->column_int64
#define systemdata_sqlite3_column_name            systemdata_sqlite3_api->column_name
#define systemdata_sqlite3_column_name16          systemdata_sqlite3_api->column_name16
#define systemdata_sqlite3_column_origin_name     systemdata_sqlite3_api->column_origin_name
#define systemdata_sqlite3_column_origin_name16   systemdata_sqlite3_api->column_origin_name16
#define systemdata_sqlite3_column_table_name      systemdata_sqlite3_api->column_table_name
#define systemdata_sqlite3_column_table_name16    systemdata_sqlite3_api->column_table_name16
#define systemdata_sqlite3_column_text            systemdata_sqlite3_api->column_text
#define systemdata_sqlite3_column_text16          systemdata_sqlite3_api->column_text16
#define systemdata_sqlite3_column_type            systemdata_sqlite3_api->column_type
#define systemdata_sqlite3_column_value           systemdata_sqlite3_api->column_value
#define systemdata_sqlite3_commit_hook            systemdata_sqlite3_api->commit_hook
#define systemdata_sqlite3_complete               systemdata_sqlite3_api->complete
#define systemdata_sqlite3_complete16             systemdata_sqlite3_api->complete16
#define systemdata_sqlite3_create_collation       systemdata_sqlite3_api->create_collation
#define systemdata_sqlite3_create_collation16     systemdata_sqlite3_api->create_collation16
#define systemdata_sqlite3_create_function        systemdata_sqlite3_api->create_function
#define systemdata_sqlite3_create_function16      systemdata_sqlite3_api->create_function16
#define systemdata_sqlite3_create_module          systemdata_sqlite3_api->create_module
#define systemdata_sqlite3_create_module_v2       systemdata_sqlite3_api->create_module_v2
#define systemdata_sqlite3_data_count             systemdata_sqlite3_api->data_count
#define systemdata_sqlite3_db_handle              systemdata_sqlite3_api->db_handle
#define systemdata_sqlite3_declare_vtab           systemdata_sqlite3_api->declare_vtab
#define systemdata_sqlite3_enable_shared_cache    systemdata_sqlite3_api->enable_shared_cache
#define systemdata_sqlite3_errcode                systemdata_sqlite3_api->errcode
#define systemdata_sqlite3_errmsg                 systemdata_sqlite3_api->errmsg
#define systemdata_sqlite3_errmsg16               systemdata_sqlite3_api->errmsg16
#define systemdata_sqlite3_exec                   systemdata_sqlite3_api->exec
#ifndef SQLITE_OMIT_DEPRECATED
#define systemdata_sqlite3_expired                systemdata_sqlite3_api->expired
#endif
#define systemdata_sqlite3_finalize               systemdata_sqlite3_api->finalize
#define systemdata_sqlite3_free                   systemdata_sqlite3_api->free
#define systemdata_sqlite3_free_table             systemdata_sqlite3_api->free_table
#define systemdata_sqlite3_get_autocommit         systemdata_sqlite3_api->get_autocommit
#define systemdata_sqlite3_get_auxdata            systemdata_sqlite3_api->get_auxdata
#define systemdata_sqlite3_get_table              systemdata_sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define systemdata_sqlite3_global_recover         systemdata_sqlite3_api->global_recover
#endif
#define systemdata_sqlite3_interrupt              systemdata_sqlite3_api->interruptx
#define systemdata_sqlite3_last_insert_rowid      systemdata_sqlite3_api->last_insert_rowid
#define systemdata_sqlite3_libversion             systemdata_sqlite3_api->libversion
#define systemdata_sqlite3_libversion_number      systemdata_sqlite3_api->libversion_number
#define systemdata_sqlite3_malloc                 systemdata_sqlite3_api->malloc
#define systemdata_sqlite3_mprintf                systemdata_sqlite3_api->mprintf
#define systemdata_sqlite3_open                   systemdata_sqlite3_api->open
#define systemdata_sqlite3_open16                 systemdata_sqlite3_api->open16
#define systemdata_sqlite3_prepare                systemdata_sqlite3_api->prepare
#define systemdata_sqlite3_prepare16              systemdata_sqlite3_api->prepare16
#define systemdata_sqlite3_prepare_v2             systemdata_sqlite3_api->prepare_v2
#define systemdata_sqlite3_prepare16_v2           systemdata_sqlite3_api->prepare16_v2
#define systemdata_sqlite3_profile                systemdata_sqlite3_api->profile
#define systemdata_sqlite3_progress_handler       systemdata_sqlite3_api->progress_handler
#define systemdata_sqlite3_realloc                systemdata_sqlite3_api->realloc
#define systemdata_sqlite3_reset                  systemdata_sqlite3_api->reset
#define systemdata_sqlite3_result_blob            systemdata_sqlite3_api->result_blob
#define systemdata_sqlite3_result_double          systemdata_sqlite3_api->result_double
#define systemdata_sqlite3_result_error           systemdata_sqlite3_api->result_error
#define systemdata_sqlite3_result_error16         systemdata_sqlite3_api->result_error16
#define systemdata_sqlite3_result_int             systemdata_sqlite3_api->result_int
#define systemdata_sqlite3_result_int64           systemdata_sqlite3_api->result_int64
#define systemdata_sqlite3_result_null            systemdata_sqlite3_api->result_null
#define systemdata_sqlite3_result_text            systemdata_sqlite3_api->result_text
#define systemdata_sqlite3_result_text16          systemdata_sqlite3_api->result_text16
#define systemdata_sqlite3_result_text16be        systemdata_sqlite3_api->result_text16be
#define systemdata_sqlite3_result_text16le        systemdata_sqlite3_api->result_text16le
#define systemdata_sqlite3_result_value           systemdata_sqlite3_api->result_value
#define systemdata_sqlite3_rollback_hook          systemdata_sqlite3_api->rollback_hook
#define systemdata_sqlite3_set_authorizer         systemdata_sqlite3_api->set_authorizer
#define systemdata_sqlite3_set_auxdata            systemdata_sqlite3_api->set_auxdata
#define systemdata_sqlite3_snprintf               systemdata_sqlite3_api->snprintf
#define systemdata_sqlite3_step                   systemdata_sqlite3_api->step
#define systemdata_sqlite3_table_column_metadata  systemdata_sqlite3_api->table_column_metadata
#define systemdata_sqlite3_thread_cleanup         systemdata_sqlite3_api->thread_cleanup
#define systemdata_sqlite3_total_changes          systemdata_sqlite3_api->total_changes
#define systemdata_sqlite3_trace                  systemdata_sqlite3_api->trace
#ifndef SQLITE_OMIT_DEPRECATED
#define systemdata_sqlite3_transfer_bindings      systemdata_sqlite3_api->transfer_bindings
#endif
#define systemdata_sqlite3_update_hook            systemdata_sqlite3_api->update_hook
#define systemdata_sqlite3_user_data              systemdata_sqlite3_api->user_data
#define systemdata_sqlite3_value_blob             systemdata_sqlite3_api->value_blob
#define systemdata_sqlite3_value_bytes            systemdata_sqlite3_api->value_bytes
#define systemdata_sqlite3_value_bytes16          systemdata_sqlite3_api->value_bytes16
#define systemdata_sqlite3_value_double           systemdata_sqlite3_api->value_double
#define systemdata_sqlite3_value_int              systemdata_sqlite3_api->value_int
#define systemdata_sqlite3_value_int64            systemdata_sqlite3_api->value_int64
#define systemdata_sqlite3_value_numeric_type     systemdata_sqlite3_api->value_numeric_type
#define systemdata_sqlite3_value_text             systemdata_sqlite3_api->value_text
#define systemdata_sqlite3_value_text16           systemdata_sqlite3_api->value_text16
#define systemdata_sqlite3_value_text16be         systemdata_sqlite3_api->value_text16be
#define systemdata_sqlite3_value_text16le         systemdata_sqlite3_api->value_text16le
#define systemdata_sqlite3_value_type             systemdata_sqlite3_api->value_type
#define systemdata_sqlite3_vmprintf               systemdata_sqlite3_api->vmprintf
#define systemdata_sqlite3_vsnprintf              systemdata_sqlite3_api->vsnprintf
#define systemdata_sqlite3_overload_function      systemdata_sqlite3_api->overload_function
#define systemdata_sqlite3_prepare_v2             systemdata_sqlite3_api->prepare_v2
#define systemdata_sqlite3_prepare16_v2           systemdata_sqlite3_api->prepare16_v2
#define systemdata_sqlite3_clear_bindings         systemdata_sqlite3_api->clear_bindings
#define systemdata_sqlite3_bind_zeroblob          systemdata_sqlite3_api->bind_zeroblob
#define systemdata_sqlite3_blob_bytes             systemdata_sqlite3_api->blob_bytes
#define systemdata_sqlite3_blob_close             systemdata_sqlite3_api->blob_close
#define systemdata_sqlite3_blob_open              systemdata_sqlite3_api->blob_open
#define systemdata_sqlite3_blob_read              systemdata_sqlite3_api->blob_read
#define systemdata_sqlite3_blob_write             systemdata_sqlite3_api->blob_write
#define systemdata_sqlite3_create_collation_v2    systemdata_sqlite3_api->create_collation_v2
#define systemdata_sqlite3_file_control           systemdata_sqlite3_api->file_control
#define systemdata_sqlite3_memory_highwater       systemdata_sqlite3_api->memory_highwater
#define systemdata_sqlite3_memory_used            systemdata_sqlite3_api->memory_used
#define systemdata_sqlite3_mutex_alloc            systemdata_sqlite3_api->mutex_alloc
#define systemdata_sqlite3_mutex_enter            systemdata_sqlite3_api->mutex_enter
#define systemdata_sqlite3_mutex_free             systemdata_sqlite3_api->mutex_free
#define systemdata_sqlite3_mutex_leave            systemdata_sqlite3_api->mutex_leave
#define systemdata_sqlite3_mutex_try              systemdata_sqlite3_api->mutex_try
#define systemdata_sqlite3_open_v2                systemdata_sqlite3_api->open_v2
#define systemdata_sqlite3_release_memory         systemdata_sqlite3_api->release_memory
#define systemdata_sqlite3_result_error_nomem     systemdata_sqlite3_api->result_error_nomem
#define systemdata_sqlite3_result_error_toobig    systemdata_sqlite3_api->result_error_toobig
#define systemdata_sqlite3_sleep                  systemdata_sqlite3_api->sleep
#define systemdata_sqlite3_soft_heap_limit        systemdata_sqlite3_api->soft_heap_limit
#define systemdata_sqlite3_vfs_find               systemdata_sqlite3_api->vfs_find
#define systemdata_sqlite3_vfs_register           systemdata_sqlite3_api->vfs_register
#define systemdata_sqlite3_vfs_unregister         systemdata_sqlite3_api->vfs_unregister
#define systemdata_sqlite3_threadsafe             systemdata_sqlite3_api->xthreadsafe
#define systemdata_sqlite3_result_zeroblob        systemdata_sqlite3_api->result_zeroblob
#define systemdata_sqlite3_result_error_code      systemdata_sqlite3_api->result_error_code
#define systemdata_sqlite3_test_control           systemdata_sqlite3_api->test_control
#define systemdata_sqlite3_randomness             systemdata_sqlite3_api->randomness
#define systemdata_sqlite3_context_db_handle      systemdata_sqlite3_api->context_db_handle
#define systemdata_sqlite3_extended_result_codes  systemdata_sqlite3_api->extended_result_codes
#define systemdata_sqlite3_limit                  systemdata_sqlite3_api->limit
#define systemdata_sqlite3_next_stmt              systemdata_sqlite3_api->next_stmt
#define systemdata_sqlite3_sql                    systemdata_sqlite3_api->sql
#define systemdata_sqlite3_status                 systemdata_sqlite3_api->status
#define systemdata_sqlite3_backup_finish          systemdata_sqlite3_api->backup_finish
#define systemdata_sqlite3_backup_init            systemdata_sqlite3_api->backup_init
#define systemdata_sqlite3_backup_pagecount       systemdata_sqlite3_api->backup_pagecount
#define systemdata_sqlite3_backup_remaining       systemdata_sqlite3_api->backup_remaining
#define systemdata_sqlite3_backup_step            systemdata_sqlite3_api->backup_step
#define systemdata_sqlite3_compileoption_get      systemdata_sqlite3_api->compileoption_get
#define systemdata_sqlite3_compileoption_used     systemdata_sqlite3_api->compileoption_used
#define systemdata_sqlite3_create_function_v2     systemdata_sqlite3_api->create_function_v2
#define systemdata_sqlite3_db_config              systemdata_sqlite3_api->db_config
#define systemdata_sqlite3_db_mutex               systemdata_sqlite3_api->db_mutex
#define systemdata_sqlite3_db_status              systemdata_sqlite3_api->db_status
#define systemdata_sqlite3_extended_errcode       systemdata_sqlite3_api->extended_errcode
#define systemdata_sqlite3_log                    systemdata_sqlite3_api->log
#define systemdata_sqlite3_soft_heap_limit64      systemdata_sqlite3_api->soft_heap_limit64
#define systemdata_sqlite3_sourceid               systemdata_sqlite3_api->sourceid
#define systemdata_sqlite3_stmt_status            systemdata_sqlite3_api->stmt_status
#define systemdata_sqlite3_strnicmp               systemdata_sqlite3_api->strnicmp
#define systemdata_sqlite3_unlock_notify          systemdata_sqlite3_api->unlock_notify
#define systemdata_sqlite3_wal_autocheckpoint     systemdata_sqlite3_api->wal_autocheckpoint
#define systemdata_sqlite3_wal_checkpoint         systemdata_sqlite3_api->wal_checkpoint
#define systemdata_sqlite3_wal_hook               systemdata_sqlite3_api->wal_hook
#define systemdata_sqlite3_blob_reopen            systemdata_sqlite3_api->blob_reopen
#define systemdata_sqlite3_vtab_config            systemdata_sqlite3_api->vtab_config
#define systemdata_sqlite3_vtab_on_conflict       systemdata_sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#define systemdata_sqlite3_close_v2               systemdata_sqlite3_api->close_v2
#define systemdata_sqlite3_db_filename            systemdata_sqlite3_api->db_filename
#define systemdata_sqlite3_db_readonly            systemdata_sqlite3_api->db_readonly
#define systemdata_sqlite3_db_release_memory      systemdata_sqlite3_api->db_release_memory
#define systemdata_sqlite3_errstr                 systemdata_sqlite3_api->errstr
#define systemdata_sqlite3_stmt_busy              systemdata_sqlite3_api->stmt_busy
#define systemdata_sqlite3_stmt_readonly          systemdata_sqlite3_api->stmt_readonly
#define systemdata_sqlite3_stricmp                systemdata_sqlite3_api->stricmp
#define systemdata_sqlite3_uri_boolean            systemdata_sqlite3_api->uri_boolean
#define systemdata_sqlite3_uri_int64              systemdata_sqlite3_api->uri_int64
#define systemdata_sqlite3_uri_parameter          systemdata_sqlite3_api->uri_parameter
#define systemdata_sqlite3_uri_vsnprintf          systemdata_sqlite3_api->vsnprintf
#define systemdata_sqlite3_wal_checkpoint_v2      systemdata_sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#define systemdata_sqlite3_auto_extension         systemdata_sqlite3_api->auto_extension
#define systemdata_sqlite3_bind_blob64            systemdata_sqlite3_api->bind_blob64
#define systemdata_sqlite3_bind_text64            systemdata_sqlite3_api->bind_text64
#define systemdata_sqlite3_cancel_auto_extension  systemdata_sqlite3_api->cancel_auto_extension
#define systemdata_sqlite3_load_extension         systemdata_sqlite3_api->load_extension
#define systemdata_sqlite3_malloc64               systemdata_sqlite3_api->malloc64
#define systemdata_sqlite3_msize                  systemdata_sqlite3_api->msize
#define systemdata_sqlite3_realloc64              systemdata_sqlite3_api->realloc64
#define systemdata_sqlite3_reset_auto_extension   systemdata_sqlite3_api->reset_auto_extension
#define systemdata_sqlite3_result_blob64          systemdata_sqlite3_api->result_blob64
#define systemdata_sqlite3_result_text64          systemdata_sqlite3_api->result_text64
#define systemdata_sqlite3_strglob                systemdata_sqlite3_api->strglob
/* Version 3.8.11 and later */
#define systemdata_sqlite3_value_dup              systemdata_sqlite3_api->value_dup
#define systemdata_sqlite3_value_free             systemdata_sqlite3_api->value_free
#define systemdata_sqlite3_result_zeroblob64      systemdata_sqlite3_api->result_zeroblob64
#define systemdata_sqlite3_bind_zeroblob64        systemdata_sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define systemdata_sqlite3_value_subtype          systemdata_sqlite3_api->value_subtype
#define systemdata_sqlite3_result_subtype         systemdata_sqlite3_api->result_subtype
/* Version 3.10.0 and later */
#define systemdata_sqlite3_status64               systemdata_sqlite3_api->status64
#define systemdata_sqlite3_strlike                systemdata_sqlite3_api->strlike
#define systemdata_sqlite3_db_cacheflush          systemdata_sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#define systemdata_sqlite3_system_errno           systemdata_sqlite3_api->system_errno
/* Version 3.14.0 and later */
#define systemdata_sqlite3_trace_v2               systemdata_sqlite3_api->trace_v2
#define systemdata_sqlite3_expanded_sql           systemdata_sqlite3_api->expanded_sql
#endif /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#if !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     const systemdata_sqlite3_api_routines *systemdata_sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  systemdata_sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    extern const systemdata_sqlite3_api_routines *systemdata_sqlite3_api;
#else
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (void)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */








