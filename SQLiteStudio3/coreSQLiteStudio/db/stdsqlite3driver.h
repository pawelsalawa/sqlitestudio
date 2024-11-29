#ifndef STDSQLITE3DRIVER_H
#define STDSQLITE3DRIVER_H

#define STD_SQLITE3_DRIVER(Name, Label, Prefix, UppercasePrefix) \
    struct API_EXPORT Name \
    { \
        static_char* label = Label; \
        \
        static const int OK = UppercasePrefix##SQLITE_OK; \
        static const int ERROR = UppercasePrefix##SQLITE_ERROR; \
        static const int OPEN_READWRITE = UppercasePrefix##SQLITE_OPEN_READWRITE; \
        static const int OPEN_CREATE = UppercasePrefix##SQLITE_OPEN_CREATE; \
        static const int UTF8 = UppercasePrefix##SQLITE_UTF8; \
        static const int DETERMINISTIC = UppercasePrefix##SQLITE_DETERMINISTIC; \
        static const int INTEGER = UppercasePrefix##SQLITE_INTEGER; \
        static const int FLOAT = UppercasePrefix##SQLITE_FLOAT; \
        static const int NULL_TYPE = UppercasePrefix##SQLITE_NULL; \
        static const int BLOB = UppercasePrefix##SQLITE_BLOB; \
        static const int MISUSE = UppercasePrefix##SQLITE_MISUSE; \
        static const int BUSY = UppercasePrefix##SQLITE_BUSY; \
        static const int ROW = UppercasePrefix##SQLITE_ROW; \
        static const int DONE = UppercasePrefix##SQLITE_DONE; \
        static const int INTERRUPT = UppercasePrefix##SQLITE_INTERRUPT; \
        static const int CHECKPOINT_PASSIVE = UppercasePrefix##SQLITE_CHECKPOINT_PASSIVE; \
        static const int CHECKPOINT_FULL = UppercasePrefix##SQLITE_CHECKPOINT_FULL; \
        static const int CHECKPOINT_RESTART = UppercasePrefix##SQLITE_CHECKPOINT_RESTART; \
        static const int CHECKPOINT_TRUNCATE = UppercasePrefix##SQLITE_CHECKPOINT_TRUNCATE; \
        \
        typedef Prefix##sqlite3 handle; \
        typedef Prefix##sqlite3_stmt stmt; \
        typedef Prefix##sqlite3_context context; \
        typedef Prefix##sqlite3_value value; \
        typedef Prefix##sqlite3_int64 int64; \
        typedef Prefix##sqlite3_destructor_type destructor_type; \
        \
        static destructor_type TRANSIENT() {return UppercasePrefix##SQLITE_TRANSIENT;} \
        static void interrupt(handle* arg) {Prefix##sqlite3_interrupt(arg);} \
        static int is_interrupted(handle* arg) {return Prefix##sqlite3_is_interrupted(arg);} \
        static const void *value_blob(value* arg) {return Prefix##sqlite3_value_blob(arg);} \
        static double value_double(value* arg) {return Prefix##sqlite3_value_double(arg);} \
        static int64 value_int64(value* arg) {return Prefix##sqlite3_value_int64(arg);} \
        static const void *value_text16(value* arg) {return Prefix##sqlite3_value_text16(arg);} \
        static int value_bytes(value* arg) {return Prefix##sqlite3_value_bytes(arg);} \
        static int value_bytes16(value* arg) {return Prefix##sqlite3_value_bytes16(arg);} \
        static int value_type(value* arg) {return Prefix##sqlite3_value_type(arg);} \
        static int bind_blob(stmt* a1, int a2, const void* a3, int a4, void(*a5)(void*)) {return Prefix##sqlite3_bind_blob(a1, a2, a3, a4, a5);} \
        static int bind_double(stmt* a1, int a2, double a3) {return Prefix##sqlite3_bind_double(a1, a2, a3);} \
        static int bind_int(stmt* a1, int a2, int a3) {return Prefix##sqlite3_bind_int(a1, a2, a3);} \
        static int bind_int64(stmt* a1, int a2, int64 a3) {return Prefix##sqlite3_bind_int64(a1, a2, a3);} \
        static int bind_null(stmt* a1, int a2) {return Prefix##sqlite3_bind_null(a1, a2);} \
        static int bind_parameter_index(stmt* a1, const char* a2) {return Prefix##sqlite3_bind_parameter_index(a1, a2);} \
        static int bind_text16(stmt* a1, int a2, const void* a3, int a4, void(*a5)(void*)) {return Prefix##sqlite3_bind_text16(a1, a2, a3, a4, a5);} \
        static void result_blob(context* a1, const void* a2, int a3, void(*a4)(void*)) {Prefix##sqlite3_result_blob(a1, a2, a3, a4);} \
        static void result_double(context* a1, double a2) {Prefix##sqlite3_result_double(a1, a2);} \
        static void result_error16(context* a1, const void* a2, int a3) {Prefix##sqlite3_result_error16(a1, a2, a3);} \
        static void result_int(context* a1, int a2) {Prefix##sqlite3_result_int(a1, a2);} \
        static void result_int64(context* a1, int64 a2) {Prefix##sqlite3_result_int64(a1, a2);} \
        static void result_null(context* a1) {Prefix##sqlite3_result_null(a1);} \
        static void result_text16(context* a1, const void* a2, int a3, void(*a4)(void*)) {Prefix##sqlite3_result_text16(a1, a2, a3, a4);} \
        static int open_v2(const char *a1, handle **a2, int a3, const char *a4) {return Prefix##sqlite3_open_v2(a1, a2, a3, a4);} \
        static int finalize(stmt *arg) {return Prefix##sqlite3_finalize(arg);} \
        static const char *errmsg(handle* arg) {return Prefix##sqlite3_errmsg(arg);} \
        static const char *column_decltype(stmt* arg1, int arg2) {return Prefix##sqlite3_column_decltype(arg1, arg2);} \
        static int column_type(stmt* arg1, int arg2) {return Prefix##sqlite3_column_type(arg1, arg2);} \
        static int extended_errcode(handle* arg) {return Prefix##sqlite3_extended_errcode(arg);} \
        static const void *column_blob(stmt* arg1, int arg2) {return Prefix##sqlite3_column_blob(arg1, arg2);} \
        static int column_bytes(stmt* arg1, int arg2) {return Prefix##sqlite3_column_bytes(arg1, arg2);} \
        static int column_bytes16(stmt* arg1, int arg2) {return Prefix##sqlite3_column_bytes16(arg1, arg2);} \
        static double column_double(stmt* arg1, int arg2) {return Prefix##sqlite3_column_double(arg1, arg2);} \
        static int64 column_int64(stmt* arg1, int arg2) {return Prefix##sqlite3_column_int64(arg1, arg2);} \
        static const void *column_text16(stmt* arg1, int arg2) {return Prefix##sqlite3_column_text16(arg1, arg2);} \
        static const char *column_name(stmt* arg1, int arg2) {return Prefix##sqlite3_column_name(arg1, arg2);} \
        static int column_count(stmt* arg1) {return Prefix##sqlite3_column_count(arg1);} \
        static const char *column_database_name(stmt* arg1, int arg2) {return Prefix##sqlite3_column_database_name(arg1, arg2);} \
        static const char *column_table_name(stmt* arg1, int arg2) {return Prefix##sqlite3_column_table_name(arg1, arg2);} \
        static const char *column_origin_name(stmt* arg1, int arg2) {return Prefix##sqlite3_column_origin_name(arg1, arg2);} \
        static int changes(handle* arg) {return Prefix##sqlite3_changes(arg);} \
        static int total_changes(handle* arg) {return Prefix##sqlite3_total_changes(arg);} \
        static int64 last_insert_rowid(handle* arg) {return Prefix##sqlite3_last_insert_rowid(arg);} \
        static int step(stmt* arg) {return Prefix##sqlite3_step(arg);} \
        static int reset(stmt* arg) {return Prefix##sqlite3_reset(arg);} \
        static int close(handle* arg) {return Prefix##sqlite3_close(arg);} \
        static void free(void* arg) {return Prefix##sqlite3_free(arg);} \
        static int threadsafe() {return Prefix##sqlite3_threadsafe();} \
        static int get_autocommit(handle* arg) {return Prefix##sqlite3_get_autocommit(arg);} \
        static int wal_checkpoint(handle* arg1, const char* arg2) {return Prefix##sqlite3_wal_checkpoint(arg1, arg2);} \
        static int wal_checkpoint_v2(handle* a1, const char* a2, int a3, int* a4, int* a5) {return Prefix##sqlite3_wal_checkpoint_v2(a1, a2, a3, a4, a5);} \
        static int enable_load_extension(handle* arg1, int arg2) {return Prefix##sqlite3_enable_load_extension(arg1, arg2);} \
        static int load_extension(handle *arg1, const char *arg2, const char *arg3, char **arg4) {return Prefix##sqlite3_load_extension(arg1, arg2, arg3, arg4);} \
        static void* user_data(context* arg) {return Prefix##sqlite3_user_data(arg);} \
        static void* aggregate_context(context* arg1, int arg2) {return Prefix##sqlite3_aggregate_context(arg1, arg2);} \
        static int collation_needed(handle* a1, void* a2, void(*a3)(void*,handle*,int eTextRep,const char*)) {return Prefix##sqlite3_collation_needed(a1, a2, a3);} \
        static int prepare_v2(handle *a1, const char *a2, int a3, stmt **a4, const char **a5) {return Prefix##sqlite3_prepare_v2(a1, a2, a3, a4, a5);} \
        static int create_function(handle *a1, const char *a2, int a3, int a4, void *a5, void (*a6)(context*,int,value**), void (*a7)(context*,int,value**), void (*a8)(context*)) \
            {return Prefix##sqlite3_create_function(a1, a2, a3, a4, a5, a6, a7, a8);} \
        static int create_function_v2(handle *a1, const char *a2, int a3, int a4, void *a5, void (*a6)(context*,int,value**), void (*a7)(context*,int,value**), void (*a8)(context*), void(*a9)(void*)) \
            {return Prefix##sqlite3_create_function_v2(a1, a2, a3, a4, a5, a6, a7, a8, a9);} \
        static int create_collation_v2(handle* a1, const char *a2, int a3, void *a4, int(*a5)(void*,int,const void*,int,const void*), void(*a6)(void*)) \
            {return Prefix##sqlite3_create_collation_v2(a1, a2, a3, a4, a5, a6);} \
        static int complete(const char* arg) {return Prefix##sqlite3_complete(arg);} \
    };

#endif // STDSQLITE3DRIVER_H
