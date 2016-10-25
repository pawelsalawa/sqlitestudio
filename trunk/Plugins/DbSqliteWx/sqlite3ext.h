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
** wx_sqlite3.h.
*/
#ifndef SQLITE3EXT_H
#define SQLITE3EXT_H
#include "wxwx_sqlite3.h"

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
struct wx_sqlite3_api_routines {
  void * (*aggregate_context)(wx_sqlite3_context*,int nBytes);
  int  (*aggregate_count)(wx_sqlite3_context*);
  int  (*bind_blob)(wx_sqlite3_stmt*,int,const void*,int n,void(*)(void*));
  int  (*bind_double)(wx_sqlite3_stmt*,int,double);
  int  (*bind_int)(wx_sqlite3_stmt*,int,int);
  int  (*bind_int64)(wx_sqlite3_stmt*,int,sqlite_int64);
  int  (*bind_null)(wx_sqlite3_stmt*,int);
  int  (*bind_parameter_count)(wx_sqlite3_stmt*);
  int  (*bind_parameter_index)(wx_sqlite3_stmt*,const char*zName);
  const char * (*bind_parameter_name)(wx_sqlite3_stmt*,int);
  int  (*bind_text)(wx_sqlite3_stmt*,int,const char*,int n,void(*)(void*));
  int  (*bind_text16)(wx_sqlite3_stmt*,int,const void*,int,void(*)(void*));
  int  (*bind_value)(wx_sqlite3_stmt*,int,const wx_sqlite3_value*);
  int  (*busy_handler)(wx_sqlite3*,int(*)(void*,int),void*);
  int  (*busy_timeout)(wx_sqlite3*,int ms);
  int  (*changes)(wx_sqlite3*);
  int  (*close)(wx_sqlite3*);
  int  (*collation_needed)(wx_sqlite3*,void*,void(*)(void*,wx_sqlite3*,
                           int eTextRep,const char*));
  int  (*collation_needed16)(wx_sqlite3*,void*,void(*)(void*,wx_sqlite3*,
                             int eTextRep,const void*));
  const void * (*column_blob)(wx_sqlite3_stmt*,int iCol);
  int  (*column_bytes)(wx_sqlite3_stmt*,int iCol);
  int  (*column_bytes16)(wx_sqlite3_stmt*,int iCol);
  int  (*column_count)(wx_sqlite3_stmt*pStmt);
  const char * (*column_database_name)(wx_sqlite3_stmt*,int);
  const void * (*column_database_name16)(wx_sqlite3_stmt*,int);
  const char * (*column_decltype)(wx_sqlite3_stmt*,int i);
  const void * (*column_decltype16)(wx_sqlite3_stmt*,int);
  double  (*column_double)(wx_sqlite3_stmt*,int iCol);
  int  (*column_int)(wx_sqlite3_stmt*,int iCol);
  sqlite_int64  (*column_int64)(wx_sqlite3_stmt*,int iCol);
  const char * (*column_name)(wx_sqlite3_stmt*,int);
  const void * (*column_name16)(wx_sqlite3_stmt*,int);
  const char * (*column_origin_name)(wx_sqlite3_stmt*,int);
  const void * (*column_origin_name16)(wx_sqlite3_stmt*,int);
  const char * (*column_table_name)(wx_sqlite3_stmt*,int);
  const void * (*column_table_name16)(wx_sqlite3_stmt*,int);
  const unsigned char * (*column_text)(wx_sqlite3_stmt*,int iCol);
  const void * (*column_text16)(wx_sqlite3_stmt*,int iCol);
  int  (*column_type)(wx_sqlite3_stmt*,int iCol);
  wx_sqlite3_value* (*column_value)(wx_sqlite3_stmt*,int iCol);
  void * (*commit_hook)(wx_sqlite3*,int(*)(void*),void*);
  int  (*complete)(const char*sql);
  int  (*complete16)(const void*sql);
  int  (*create_collation)(wx_sqlite3*,const char*,int,void*,
                           int(*)(void*,int,const void*,int,const void*));
  int  (*create_collation16)(wx_sqlite3*,const void*,int,void*,
                             int(*)(void*,int,const void*,int,const void*));
  int  (*create_function)(wx_sqlite3*,const char*,int,int,void*,
                          void (*xFunc)(wx_sqlite3_context*,int,wx_sqlite3_value**),
                          void (*xStep)(wx_sqlite3_context*,int,wx_sqlite3_value**),
                          void (*xFinal)(wx_sqlite3_context*));
  int  (*create_function16)(wx_sqlite3*,const void*,int,int,void*,
                            void (*xFunc)(wx_sqlite3_context*,int,wx_sqlite3_value**),
                            void (*xStep)(wx_sqlite3_context*,int,wx_sqlite3_value**),
                            void (*xFinal)(wx_sqlite3_context*));
  int (*create_module)(wx_sqlite3*,const char*,const wx_sqlite3_module*,void*);
  int  (*data_count)(wx_sqlite3_stmt*pStmt);
  wx_sqlite3 * (*db_handle)(wx_sqlite3_stmt*);
  int (*declare_vtab)(wx_sqlite3*,const char*);
  int  (*enable_shared_cache)(int);
  int  (*errcode)(wx_sqlite3*db);
  const char * (*errmsg)(wx_sqlite3*);
  const void * (*errmsg16)(wx_sqlite3*);
  int  (*exec)(wx_sqlite3*,const char*,wx_sqlite3_callback,void*,char**);
  int  (*expired)(wx_sqlite3_stmt*);
  int  (*finalize)(wx_sqlite3_stmt*pStmt);
  void  (*free)(void*);
  void  (*free_table)(char**result);
  int  (*get_autocommit)(wx_sqlite3*);
  void * (*get_auxdata)(wx_sqlite3_context*,int);
  int  (*get_table)(wx_sqlite3*,const char*,char***,int*,int*,char**);
  int  (*global_recover)(void);
  void  (*interruptx)(wx_sqlite3*);
  sqlite_int64  (*last_insert_rowid)(wx_sqlite3*);
  const char * (*libversion)(void);
  int  (*libversion_number)(void);
  void *(*malloc)(int);
  char * (*mprintf)(const char*,...);
  int  (*open)(const char*,wx_sqlite3**);
  int  (*open16)(const void*,wx_sqlite3**);
  int  (*prepare)(wx_sqlite3*,const char*,int,wx_sqlite3_stmt**,const char**);
  int  (*prepare16)(wx_sqlite3*,const void*,int,wx_sqlite3_stmt**,const void**);
  void * (*profile)(wx_sqlite3*,void(*)(void*,const char*,sqlite_uint64),void*);
  void  (*progress_handler)(wx_sqlite3*,int,int(*)(void*),void*);
  void *(*realloc)(void*,int);
  int  (*reset)(wx_sqlite3_stmt*pStmt);
  void  (*result_blob)(wx_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_double)(wx_sqlite3_context*,double);
  void  (*result_error)(wx_sqlite3_context*,const char*,int);
  void  (*result_error16)(wx_sqlite3_context*,const void*,int);
  void  (*result_int)(wx_sqlite3_context*,int);
  void  (*result_int64)(wx_sqlite3_context*,sqlite_int64);
  void  (*result_null)(wx_sqlite3_context*);
  void  (*result_text)(wx_sqlite3_context*,const char*,int,void(*)(void*));
  void  (*result_text16)(wx_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_text16be)(wx_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_text16le)(wx_sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_value)(wx_sqlite3_context*,wx_sqlite3_value*);
  void * (*rollback_hook)(wx_sqlite3*,void(*)(void*),void*);
  int  (*set_authorizer)(wx_sqlite3*,int(*)(void*,int,const char*,const char*,
                         const char*,const char*),void*);
  void  (*set_auxdata)(wx_sqlite3_context*,int,void*,void (*)(void*));
  char * (*snprintf)(int,char*,const char*,...);
  int  (*step)(wx_sqlite3_stmt*);
  int  (*table_column_metadata)(wx_sqlite3*,const char*,const char*,const char*,
                                char const**,char const**,int*,int*,int*);
  void  (*thread_cleanup)(void);
  int  (*total_changes)(wx_sqlite3*);
  void * (*trace)(wx_sqlite3*,void(*xTrace)(void*,const char*),void*);
  int  (*transfer_bindings)(wx_sqlite3_stmt*,wx_sqlite3_stmt*);
  void * (*update_hook)(wx_sqlite3*,void(*)(void*,int ,char const*,char const*,
                                         sqlite_int64),void*);
  void * (*user_data)(wx_sqlite3_context*);
  const void * (*value_blob)(wx_sqlite3_value*);
  int  (*value_bytes)(wx_sqlite3_value*);
  int  (*value_bytes16)(wx_sqlite3_value*);
  double  (*value_double)(wx_sqlite3_value*);
  int  (*value_int)(wx_sqlite3_value*);
  sqlite_int64  (*value_int64)(wx_sqlite3_value*);
  int  (*value_numeric_type)(wx_sqlite3_value*);
  const unsigned char * (*value_text)(wx_sqlite3_value*);
  const void * (*value_text16)(wx_sqlite3_value*);
  const void * (*value_text16be)(wx_sqlite3_value*);
  const void * (*value_text16le)(wx_sqlite3_value*);
  int  (*value_type)(wx_sqlite3_value*);
  char *(*vmprintf)(const char*,va_list);
  /* Added ??? */
  int (*overload_function)(wx_sqlite3*, const char *zFuncName, int nArg);
  /* Added by 3.3.13 */
  int (*prepare_v2)(wx_sqlite3*,const char*,int,wx_sqlite3_stmt**,const char**);
  int (*prepare16_v2)(wx_sqlite3*,const void*,int,wx_sqlite3_stmt**,const void**);
  int (*clear_bindings)(wx_sqlite3_stmt*);
  /* Added by 3.4.1 */
  int (*create_module_v2)(wx_sqlite3*,const char*,const wx_sqlite3_module*,void*,
                          void (*xDestroy)(void *));
  /* Added by 3.5.0 */
  int (*bind_zeroblob)(wx_sqlite3_stmt*,int,int);
  int (*blob_bytes)(wx_sqlite3_blob*);
  int (*blob_close)(wx_sqlite3_blob*);
  int (*blob_open)(wx_sqlite3*,const char*,const char*,const char*,wx_sqlite3_int64,
                   int,wx_sqlite3_blob**);
  int (*blob_read)(wx_sqlite3_blob*,void*,int,int);
  int (*blob_write)(wx_sqlite3_blob*,const void*,int,int);
  int (*create_collation_v2)(wx_sqlite3*,const char*,int,void*,
                             int(*)(void*,int,const void*,int,const void*),
                             void(*)(void*));
  int (*file_control)(wx_sqlite3*,const char*,int,void*);
  wx_sqlite3_int64 (*memory_highwater)(int);
  wx_sqlite3_int64 (*memory_used)(void);
  wx_sqlite3_mutex *(*mutex_alloc)(int);
  void (*mutex_enter)(wx_sqlite3_mutex*);
  void (*mutex_free)(wx_sqlite3_mutex*);
  void (*mutex_leave)(wx_sqlite3_mutex*);
  int (*mutex_try)(wx_sqlite3_mutex*);
  int (*open_v2)(const char*,wx_sqlite3**,int,const char*);
  int (*release_memory)(int);
  void (*result_error_nomem)(wx_sqlite3_context*);
  void (*result_error_toobig)(wx_sqlite3_context*);
  int (*sleep)(int);
  void (*soft_heap_limit)(int);
  wx_sqlite3_vfs *(*vfs_find)(const char*);
  int (*vfs_register)(wx_sqlite3_vfs*,int);
  int (*vfs_unregister)(wx_sqlite3_vfs*);
  int (*xthreadsafe)(void);
  void (*result_zeroblob)(wx_sqlite3_context*,int);
  void (*result_error_code)(wx_sqlite3_context*,int);
  int (*test_control)(int, ...);
  void (*randomness)(int,void*);
  wx_sqlite3 *(*context_db_handle)(wx_sqlite3_context*);
  int (*extended_result_codes)(wx_sqlite3*,int);
  int (*limit)(wx_sqlite3*,int,int);
  wx_sqlite3_stmt *(*next_stmt)(wx_sqlite3*,wx_sqlite3_stmt*);
  const char *(*sql)(wx_sqlite3_stmt*);
  int (*status)(int,int*,int*,int);
  int (*backup_finish)(wx_sqlite3_backup*);
  wx_sqlite3_backup *(*backup_init)(wx_sqlite3*,const char*,wx_sqlite3*,const char*);
  int (*backup_pagecount)(wx_sqlite3_backup*);
  int (*backup_remaining)(wx_sqlite3_backup*);
  int (*backup_step)(wx_sqlite3_backup*,int);
  const char *(*compileoption_get)(int);
  int (*compileoption_used)(const char*);
  int (*create_function_v2)(wx_sqlite3*,const char*,int,int,void*,
                            void (*xFunc)(wx_sqlite3_context*,int,wx_sqlite3_value**),
                            void (*xStep)(wx_sqlite3_context*,int,wx_sqlite3_value**),
                            void (*xFinal)(wx_sqlite3_context*),
                            void(*xDestroy)(void*));
  int (*db_config)(wx_sqlite3*,int,...);
  wx_sqlite3_mutex *(*db_mutex)(wx_sqlite3*);
  int (*db_status)(wx_sqlite3*,int,int*,int*,int);
  int (*extended_errcode)(wx_sqlite3*);
  void (*log)(int,const char*,...);
  wx_sqlite3_int64 (*soft_heap_limit64)(wx_sqlite3_int64);
  const char *(*sourceid)(void);
  int (*stmt_status)(wx_sqlite3_stmt*,int,int);
  int (*strnicmp)(const char*,const char*,int);
  int (*unlock_notify)(wx_sqlite3*,void(*)(void**,int),void*);
  int (*wal_autocheckpoint)(wx_sqlite3*,int);
  int (*wal_checkpoint)(wx_sqlite3*,const char*);
  void *(*wal_hook)(wx_sqlite3*,int(*)(void*,wx_sqlite3*,const char*,int),void*);
  int (*blob_reopen)(wx_sqlite3_blob*,wx_sqlite3_int64);
  int (*vtab_config)(wx_sqlite3*,int op,...);
  int (*vtab_on_conflict)(wx_sqlite3*);
  /* Version 3.7.16 and later */
  int (*close_v2)(wx_sqlite3*);
  const char *(*db_filename)(wx_sqlite3*,const char*);
  int (*db_readonly)(wx_sqlite3*,const char*);
  int (*db_release_memory)(wx_sqlite3*);
  const char *(*errstr)(int);
  int (*stmt_busy)(wx_sqlite3_stmt*);
  int (*stmt_readonly)(wx_sqlite3_stmt*);
  int (*stricmp)(const char*,const char*);
  int (*uri_boolean)(const char*,const char*,int);
  wx_sqlite3_int64 (*uri_int64)(const char*,const char*,wx_sqlite3_int64);
  const char *(*uri_parameter)(const char*,const char*);
  char *(*vsnprintf)(int,char*,const char*,va_list);
  int (*wal_checkpoint_v2)(wx_sqlite3*,const char*,int,int*,int*);
  /* Version 3.8.7 and later */
  int (*auto_extension)(void(*)(void));
  int (*bind_blob64)(wx_sqlite3_stmt*,int,const void*,wx_sqlite3_uint64,
                     void(*)(void*));
  int (*bind_text64)(wx_sqlite3_stmt*,int,const char*,wx_sqlite3_uint64,
                      void(*)(void*),unsigned char);
  int (*cancel_auto_extension)(void(*)(void));
  int (*load_extension)(wx_sqlite3*,const char*,const char*,char**);
  void *(*malloc64)(wx_sqlite3_uint64);
  wx_sqlite3_uint64 (*msize)(void*);
  void *(*realloc64)(void*,wx_sqlite3_uint64);
  void (*reset_auto_extension)(void);
  void (*result_blob64)(wx_sqlite3_context*,const void*,wx_sqlite3_uint64,
                        void(*)(void*));
  void (*result_text64)(wx_sqlite3_context*,const char*,wx_sqlite3_uint64,
                         void(*)(void*), unsigned char);
  int (*strglob)(const char*,const char*);
  /* Version 3.8.11 and later */
  wx_sqlite3_value *(*value_dup)(const wx_sqlite3_value*);
  void (*value_free)(wx_sqlite3_value*);
  int (*result_zeroblob64)(wx_sqlite3_context*,wx_sqlite3_uint64);
  int (*bind_zeroblob64)(wx_sqlite3_stmt*, int, wx_sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned int (*value_subtype)(wx_sqlite3_value*);
  void (*result_subtype)(wx_sqlite3_context*,unsigned int);
  /* Version 3.10.0 and later */
  int (*status64)(int,wx_sqlite3_int64*,wx_sqlite3_int64*,int);
  int (*strlike)(const char*,const char*,unsigned int);
  int (*db_cacheflush)(wx_sqlite3*);
  /* Version 3.12.0 and later */
  int (*system_errno)(wx_sqlite3*);
  /* Version 3.14.0 and later */
  int (*trace_v2)(wx_sqlite3*,unsigned,int(*)(unsigned,void*,void*,void*),void*);
  char *(*expanded_sql)(wx_sqlite3_stmt*);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
typedef int (*wx_sqlite3_loadext_entry)(
  wx_sqlite3 *db,                       /* Handle to the database. */
  char **pzErrMsg,                   /* Used to set error string on failure. */
  const wx_sqlite3_api_routines *pThunk /* Extension API function pointers. */
);

/*
** The following macros redefine the API routines so that they are
** redirected through the global wx_sqlite3_api structure.
**
** This header file is also used by the loadext.c source file
** (part of the main SQLite library - not an extension) so that
** it can get access to the wx_sqlite3_api_routines structure
** definition.  But the main library does not want to redefine
** the API.  So the redefinition macros are only valid if the
** SQLITE_CORE macros is undefined.
*/
#if !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define wx_sqlite3_aggregate_context      wx_sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define wx_sqlite3_aggregate_count        wx_sqlite3_api->aggregate_count
#endif
#define wx_sqlite3_bind_blob              wx_sqlite3_api->bind_blob
#define wx_sqlite3_bind_double            wx_sqlite3_api->bind_double
#define wx_sqlite3_bind_int               wx_sqlite3_api->bind_int
#define wx_sqlite3_bind_int64             wx_sqlite3_api->bind_int64
#define wx_sqlite3_bind_null              wx_sqlite3_api->bind_null
#define wx_sqlite3_bind_parameter_count   wx_sqlite3_api->bind_parameter_count
#define wx_sqlite3_bind_parameter_index   wx_sqlite3_api->bind_parameter_index
#define wx_sqlite3_bind_parameter_name    wx_sqlite3_api->bind_parameter_name
#define wx_sqlite3_bind_text              wx_sqlite3_api->bind_text
#define wx_sqlite3_bind_text16            wx_sqlite3_api->bind_text16
#define wx_sqlite3_bind_value             wx_sqlite3_api->bind_value
#define wx_sqlite3_busy_handler           wx_sqlite3_api->busy_handler
#define wx_sqlite3_busy_timeout           wx_sqlite3_api->busy_timeout
#define wx_sqlite3_changes                wx_sqlite3_api->changes
#define wx_sqlite3_close                  wx_sqlite3_api->close
#define wx_sqlite3_collation_needed       wx_sqlite3_api->collation_needed
#define wx_sqlite3_collation_needed16     wx_sqlite3_api->collation_needed16
#define wx_sqlite3_column_blob            wx_sqlite3_api->column_blob
#define wx_sqlite3_column_bytes           wx_sqlite3_api->column_bytes
#define wx_sqlite3_column_bytes16         wx_sqlite3_api->column_bytes16
#define wx_sqlite3_column_count           wx_sqlite3_api->column_count
#define wx_sqlite3_column_database_name   wx_sqlite3_api->column_database_name
#define wx_sqlite3_column_database_name16 wx_sqlite3_api->column_database_name16
#define wx_sqlite3_column_decltype        wx_sqlite3_api->column_decltype
#define wx_sqlite3_column_decltype16      wx_sqlite3_api->column_decltype16
#define wx_sqlite3_column_double          wx_sqlite3_api->column_double
#define wx_sqlite3_column_int             wx_sqlite3_api->column_int
#define wx_sqlite3_column_int64           wx_sqlite3_api->column_int64
#define wx_sqlite3_column_name            wx_sqlite3_api->column_name
#define wx_sqlite3_column_name16          wx_sqlite3_api->column_name16
#define wx_sqlite3_column_origin_name     wx_sqlite3_api->column_origin_name
#define wx_sqlite3_column_origin_name16   wx_sqlite3_api->column_origin_name16
#define wx_sqlite3_column_table_name      wx_sqlite3_api->column_table_name
#define wx_sqlite3_column_table_name16    wx_sqlite3_api->column_table_name16
#define wx_sqlite3_column_text            wx_sqlite3_api->column_text
#define wx_sqlite3_column_text16          wx_sqlite3_api->column_text16
#define wx_sqlite3_column_type            wx_sqlite3_api->column_type
#define wx_sqlite3_column_value           wx_sqlite3_api->column_value
#define wx_sqlite3_commit_hook            wx_sqlite3_api->commit_hook
#define wx_sqlite3_complete               wx_sqlite3_api->complete
#define wx_sqlite3_complete16             wx_sqlite3_api->complete16
#define wx_sqlite3_create_collation       wx_sqlite3_api->create_collation
#define wx_sqlite3_create_collation16     wx_sqlite3_api->create_collation16
#define wx_sqlite3_create_function        wx_sqlite3_api->create_function
#define wx_sqlite3_create_function16      wx_sqlite3_api->create_function16
#define wx_sqlite3_create_module          wx_sqlite3_api->create_module
#define wx_sqlite3_create_module_v2       wx_sqlite3_api->create_module_v2
#define wx_sqlite3_data_count             wx_sqlite3_api->data_count
#define wx_sqlite3_db_handle              wx_sqlite3_api->db_handle
#define wx_sqlite3_declare_vtab           wx_sqlite3_api->declare_vtab
#define wx_sqlite3_enable_shared_cache    wx_sqlite3_api->enable_shared_cache
#define wx_sqlite3_errcode                wx_sqlite3_api->errcode
#define wx_sqlite3_errmsg                 wx_sqlite3_api->errmsg
#define wx_sqlite3_errmsg16               wx_sqlite3_api->errmsg16
#define wx_sqlite3_exec                   wx_sqlite3_api->exec
#ifndef SQLITE_OMIT_DEPRECATED
#define wx_sqlite3_expired                wx_sqlite3_api->expired
#endif
#define wx_sqlite3_finalize               wx_sqlite3_api->finalize
#define wx_sqlite3_free                   wx_sqlite3_api->free
#define wx_sqlite3_free_table             wx_sqlite3_api->free_table
#define wx_sqlite3_get_autocommit         wx_sqlite3_api->get_autocommit
#define wx_sqlite3_get_auxdata            wx_sqlite3_api->get_auxdata
#define wx_sqlite3_get_table              wx_sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define wx_sqlite3_global_recover         wx_sqlite3_api->global_recover
#endif
#define wx_sqlite3_interrupt              wx_sqlite3_api->interruptx
#define wx_sqlite3_last_insert_rowid      wx_sqlite3_api->last_insert_rowid
#define wx_sqlite3_libversion             wx_sqlite3_api->libversion
#define wx_sqlite3_libversion_number      wx_sqlite3_api->libversion_number
#define wx_sqlite3_malloc                 wx_sqlite3_api->malloc
#define wx_sqlite3_mprintf                wx_sqlite3_api->mprintf
#define wx_sqlite3_open                   wx_sqlite3_api->open
#define wx_sqlite3_open16                 wx_sqlite3_api->open16
#define wx_sqlite3_prepare                wx_sqlite3_api->prepare
#define wx_sqlite3_prepare16              wx_sqlite3_api->prepare16
#define wx_sqlite3_prepare_v2             wx_sqlite3_api->prepare_v2
#define wx_sqlite3_prepare16_v2           wx_sqlite3_api->prepare16_v2
#define wx_sqlite3_profile                wx_sqlite3_api->profile
#define wx_sqlite3_progress_handler       wx_sqlite3_api->progress_handler
#define wx_sqlite3_realloc                wx_sqlite3_api->realloc
#define wx_sqlite3_reset                  wx_sqlite3_api->reset
#define wx_sqlite3_result_blob            wx_sqlite3_api->result_blob
#define wx_sqlite3_result_double          wx_sqlite3_api->result_double
#define wx_sqlite3_result_error           wx_sqlite3_api->result_error
#define wx_sqlite3_result_error16         wx_sqlite3_api->result_error16
#define wx_sqlite3_result_int             wx_sqlite3_api->result_int
#define wx_sqlite3_result_int64           wx_sqlite3_api->result_int64
#define wx_sqlite3_result_null            wx_sqlite3_api->result_null
#define wx_sqlite3_result_text            wx_sqlite3_api->result_text
#define wx_sqlite3_result_text16          wx_sqlite3_api->result_text16
#define wx_sqlite3_result_text16be        wx_sqlite3_api->result_text16be
#define wx_sqlite3_result_text16le        wx_sqlite3_api->result_text16le
#define wx_sqlite3_result_value           wx_sqlite3_api->result_value
#define wx_sqlite3_rollback_hook          wx_sqlite3_api->rollback_hook
#define wx_sqlite3_set_authorizer         wx_sqlite3_api->set_authorizer
#define wx_sqlite3_set_auxdata            wx_sqlite3_api->set_auxdata
#define wx_sqlite3_snprintf               wx_sqlite3_api->snprintf
#define wx_sqlite3_step                   wx_sqlite3_api->step
#define wx_sqlite3_table_column_metadata  wx_sqlite3_api->table_column_metadata
#define wx_sqlite3_thread_cleanup         wx_sqlite3_api->thread_cleanup
#define wx_sqlite3_total_changes          wx_sqlite3_api->total_changes
#define wx_sqlite3_trace                  wx_sqlite3_api->trace
#ifndef SQLITE_OMIT_DEPRECATED
#define wx_sqlite3_transfer_bindings      wx_sqlite3_api->transfer_bindings
#endif
#define wx_sqlite3_update_hook            wx_sqlite3_api->update_hook
#define wx_sqlite3_user_data              wx_sqlite3_api->user_data
#define wx_sqlite3_value_blob             wx_sqlite3_api->value_blob
#define wx_sqlite3_value_bytes            wx_sqlite3_api->value_bytes
#define wx_sqlite3_value_bytes16          wx_sqlite3_api->value_bytes16
#define wx_sqlite3_value_double           wx_sqlite3_api->value_double
#define wx_sqlite3_value_int              wx_sqlite3_api->value_int
#define wx_sqlite3_value_int64            wx_sqlite3_api->value_int64
#define wx_sqlite3_value_numeric_type     wx_sqlite3_api->value_numeric_type
#define wx_sqlite3_value_text             wx_sqlite3_api->value_text
#define wx_sqlite3_value_text16           wx_sqlite3_api->value_text16
#define wx_sqlite3_value_text16be         wx_sqlite3_api->value_text16be
#define wx_sqlite3_value_text16le         wx_sqlite3_api->value_text16le
#define wx_sqlite3_value_type             wx_sqlite3_api->value_type
#define wx_sqlite3_vmprintf               wx_sqlite3_api->vmprintf
#define wx_sqlite3_vsnprintf              wx_sqlite3_api->vsnprintf
#define wx_sqlite3_overload_function      wx_sqlite3_api->overload_function
#define wx_sqlite3_prepare_v2             wx_sqlite3_api->prepare_v2
#define wx_sqlite3_prepare16_v2           wx_sqlite3_api->prepare16_v2
#define wx_sqlite3_clear_bindings         wx_sqlite3_api->clear_bindings
#define wx_sqlite3_bind_zeroblob          wx_sqlite3_api->bind_zeroblob
#define wx_sqlite3_blob_bytes             wx_sqlite3_api->blob_bytes
#define wx_sqlite3_blob_close             wx_sqlite3_api->blob_close
#define wx_sqlite3_blob_open              wx_sqlite3_api->blob_open
#define wx_sqlite3_blob_read              wx_sqlite3_api->blob_read
#define wx_sqlite3_blob_write             wx_sqlite3_api->blob_write
#define wx_sqlite3_create_collation_v2    wx_sqlite3_api->create_collation_v2
#define wx_sqlite3_file_control           wx_sqlite3_api->file_control
#define wx_sqlite3_memory_highwater       wx_sqlite3_api->memory_highwater
#define wx_sqlite3_memory_used            wx_sqlite3_api->memory_used
#define wx_sqlite3_mutex_alloc            wx_sqlite3_api->mutex_alloc
#define wx_sqlite3_mutex_enter            wx_sqlite3_api->mutex_enter
#define wx_sqlite3_mutex_free             wx_sqlite3_api->mutex_free
#define wx_sqlite3_mutex_leave            wx_sqlite3_api->mutex_leave
#define wx_sqlite3_mutex_try              wx_sqlite3_api->mutex_try
#define wx_sqlite3_open_v2                wx_sqlite3_api->open_v2
#define wx_sqlite3_release_memory         wx_sqlite3_api->release_memory
#define wx_sqlite3_result_error_nomem     wx_sqlite3_api->result_error_nomem
#define wx_sqlite3_result_error_toobig    wx_sqlite3_api->result_error_toobig
#define wx_sqlite3_sleep                  wx_sqlite3_api->sleep
#define wx_sqlite3_soft_heap_limit        wx_sqlite3_api->soft_heap_limit
#define wx_sqlite3_vfs_find               wx_sqlite3_api->vfs_find
#define wx_sqlite3_vfs_register           wx_sqlite3_api->vfs_register
#define wx_sqlite3_vfs_unregister         wx_sqlite3_api->vfs_unregister
#define wx_sqlite3_threadsafe             wx_sqlite3_api->xthreadsafe
#define wx_sqlite3_result_zeroblob        wx_sqlite3_api->result_zeroblob
#define wx_sqlite3_result_error_code      wx_sqlite3_api->result_error_code
#define wx_sqlite3_test_control           wx_sqlite3_api->test_control
#define wx_sqlite3_randomness             wx_sqlite3_api->randomness
#define wx_sqlite3_context_db_handle      wx_sqlite3_api->context_db_handle
#define wx_sqlite3_extended_result_codes  wx_sqlite3_api->extended_result_codes
#define wx_sqlite3_limit                  wx_sqlite3_api->limit
#define wx_sqlite3_next_stmt              wx_sqlite3_api->next_stmt
#define wx_sqlite3_sql                    wx_sqlite3_api->sql
#define wx_sqlite3_status                 wx_sqlite3_api->status
#define wx_sqlite3_backup_finish          wx_sqlite3_api->backup_finish
#define wx_sqlite3_backup_init            wx_sqlite3_api->backup_init
#define wx_sqlite3_backup_pagecount       wx_sqlite3_api->backup_pagecount
#define wx_sqlite3_backup_remaining       wx_sqlite3_api->backup_remaining
#define wx_sqlite3_backup_step            wx_sqlite3_api->backup_step
#define wx_sqlite3_compileoption_get      wx_sqlite3_api->compileoption_get
#define wx_sqlite3_compileoption_used     wx_sqlite3_api->compileoption_used
#define wx_sqlite3_create_function_v2     wx_sqlite3_api->create_function_v2
#define wx_sqlite3_db_config              wx_sqlite3_api->db_config
#define wx_sqlite3_db_mutex               wx_sqlite3_api->db_mutex
#define wx_sqlite3_db_status              wx_sqlite3_api->db_status
#define wx_sqlite3_extended_errcode       wx_sqlite3_api->extended_errcode
#define wx_sqlite3_log                    wx_sqlite3_api->log
#define wx_sqlite3_soft_heap_limit64      wx_sqlite3_api->soft_heap_limit64
#define wx_sqlite3_sourceid               wx_sqlite3_api->sourceid
#define wx_sqlite3_stmt_status            wx_sqlite3_api->stmt_status
#define wx_sqlite3_strnicmp               wx_sqlite3_api->strnicmp
#define wx_sqlite3_unlock_notify          wx_sqlite3_api->unlock_notify
#define wx_sqlite3_wal_autocheckpoint     wx_sqlite3_api->wal_autocheckpoint
#define wx_sqlite3_wal_checkpoint         wx_sqlite3_api->wal_checkpoint
#define wx_sqlite3_wal_hook               wx_sqlite3_api->wal_hook
#define wx_sqlite3_blob_reopen            wx_sqlite3_api->blob_reopen
#define wx_sqlite3_vtab_config            wx_sqlite3_api->vtab_config
#define wx_sqlite3_vtab_on_conflict       wx_sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#define wx_sqlite3_close_v2               wx_sqlite3_api->close_v2
#define wx_sqlite3_db_filename            wx_sqlite3_api->db_filename
#define wx_sqlite3_db_readonly            wx_sqlite3_api->db_readonly
#define wx_sqlite3_db_release_memory      wx_sqlite3_api->db_release_memory
#define wx_sqlite3_errstr                 wx_sqlite3_api->errstr
#define wx_sqlite3_stmt_busy              wx_sqlite3_api->stmt_busy
#define wx_sqlite3_stmt_readonly          wx_sqlite3_api->stmt_readonly
#define wx_sqlite3_stricmp                wx_sqlite3_api->stricmp
#define wx_sqlite3_uri_boolean            wx_sqlite3_api->uri_boolean
#define wx_sqlite3_uri_int64              wx_sqlite3_api->uri_int64
#define wx_sqlite3_uri_parameter          wx_sqlite3_api->uri_parameter
#define wx_sqlite3_uri_vsnprintf          wx_sqlite3_api->vsnprintf
#define wx_sqlite3_wal_checkpoint_v2      wx_sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#define wx_sqlite3_auto_extension         wx_sqlite3_api->auto_extension
#define wx_sqlite3_bind_blob64            wx_sqlite3_api->bind_blob64
#define wx_sqlite3_bind_text64            wx_sqlite3_api->bind_text64
#define wx_sqlite3_cancel_auto_extension  wx_sqlite3_api->cancel_auto_extension
#define wx_sqlite3_load_extension         wx_sqlite3_api->load_extension
#define wx_sqlite3_malloc64               wx_sqlite3_api->malloc64
#define wx_sqlite3_msize                  wx_sqlite3_api->msize
#define wx_sqlite3_realloc64              wx_sqlite3_api->realloc64
#define wx_sqlite3_reset_auto_extension   wx_sqlite3_api->reset_auto_extension
#define wx_sqlite3_result_blob64          wx_sqlite3_api->result_blob64
#define wx_sqlite3_result_text64          wx_sqlite3_api->result_text64
#define wx_sqlite3_strglob                wx_sqlite3_api->strglob
/* Version 3.8.11 and later */
#define wx_sqlite3_value_dup              wx_sqlite3_api->value_dup
#define wx_sqlite3_value_free             wx_sqlite3_api->value_free
#define wx_sqlite3_result_zeroblob64      wx_sqlite3_api->result_zeroblob64
#define wx_sqlite3_bind_zeroblob64        wx_sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define wx_sqlite3_value_subtype          wx_sqlite3_api->value_subtype
#define wx_sqlite3_result_subtype         wx_sqlite3_api->result_subtype
/* Version 3.10.0 and later */
#define wx_sqlite3_status64               wx_sqlite3_api->status64
#define wx_sqlite3_strlike                wx_sqlite3_api->strlike
#define wx_sqlite3_db_cacheflush          wx_sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#define wx_sqlite3_system_errno           wx_sqlite3_api->system_errno
/* Version 3.14.0 and later */
#define wx_sqlite3_trace_v2               wx_sqlite3_api->trace_v2
#define wx_sqlite3_expanded_sql           wx_sqlite3_api->expanded_sql
#endif /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#if !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     const wx_sqlite3_api_routines *wx_sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  wx_sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    extern const wx_sqlite3_api_routines *wx_sqlite3_api;
#else
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (void)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */

