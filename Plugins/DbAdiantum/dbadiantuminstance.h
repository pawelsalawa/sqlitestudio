#ifndef DBADIANTUMINSTANCE_H
#define DBADIANTUMINSTANCE_H

#include <sqlite3.h>
#include "db/abstractdb3.h"
#include "dbadiantum_global.h"
#include <QRegularExpression>

// Adiantum driver - wraps SQLite with Adiantum VFS
struct AdiantumDriver
{
    static const char* label;

    // SQLite type typedefs
    typedef sqlite3 handle;
    typedef sqlite3_stmt stmt;
    typedef sqlite3_context context;
    typedef sqlite3_value value;
    typedef sqlite3_int64 int64;
    typedef sqlite3_destructor_type destructor_type;

    // SQLite constants
    static const int OK = SQLITE_OK;
    static const int ERROR = SQLITE_ERROR;
    static const int OPEN_READWRITE = SQLITE_OPEN_READWRITE;
    static const int OPEN_CREATE = SQLITE_OPEN_CREATE;
    static const int UTF8 = SQLITE_UTF8;
    static const int DETERMINISTIC = SQLITE_DETERMINISTIC;
    static const int INTEGER = SQLITE_INTEGER;
    static const int FLOAT = SQLITE_FLOAT;
    static const int NULL_TYPE = SQLITE_NULL;
    static const int BLOB = SQLITE_BLOB;
    static const int MISUSE = SQLITE_MISUSE;
    static const int BUSY = SQLITE_BUSY;
    static const int ROW = SQLITE_ROW;
    static const int DONE = SQLITE_DONE;
    static const int INTERRUPT = SQLITE_INTERRUPT;
    static const int CHECKPOINT_PASSIVE = SQLITE_CHECKPOINT_PASSIVE;
    static const int CHECKPOINT_FULL = SQLITE_CHECKPOINT_FULL;
    static const int CHECKPOINT_RESTART = SQLITE_CHECKPOINT_RESTART;
    static const int CHECKPOINT_TRUNCATE = SQLITE_CHECKPOINT_TRUNCATE;
    static const int DBCONFIG_ENABLE_LOAD_EXTENSION = SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION;

    static destructor_type TRANSIENT() {return SQLITE_TRANSIENT;}
    static void interrupt(handle* arg) {sqlite3_interrupt(arg);}
    static int is_interrupted(handle* arg) {return sqlite3_is_interrupted(arg);}
    static const void* value_blob(value* arg) {return sqlite3_value_blob(arg);}
    static double value_double(value* arg) {return sqlite3_value_double(arg);}
    static int64 value_int64(value* arg) {return sqlite3_value_int64(arg);}
    static const void* value_text16(value* arg) {return sqlite3_value_text16(arg);}
    static int value_bytes(value* arg) {return sqlite3_value_bytes(arg);}
    static int value_bytes16(value* arg) {return sqlite3_value_bytes16(arg);}
    static int value_type(value* arg) {return sqlite3_value_type(arg);}
    static int bind_blob(stmt* a1, int a2, const void* a3, int a4, void(*a5)(void*)) {return sqlite3_bind_blob(a1, a2, a3, a4, a5);}
    static int bind_double(stmt* a1, int a2, double a3) {return sqlite3_bind_double(a1, a2, a3);}
    static int bind_int(stmt* a1, int a2, int a3) {return sqlite3_bind_int(a1, a2, a3);}
    static int bind_int64(stmt* a1, int a2, int64 a3) {return sqlite3_bind_int64(a1, a2, a3);}
    static int bind_null(stmt* a1, int a2) {return sqlite3_bind_null(a1, a2);}
    static int bind_parameter_index(stmt* a1, const char* a2) {return sqlite3_bind_parameter_index(a1, a2);}
    static int bind_text16(stmt* a1, int a2, const void* a3, int a4, void(*a5)(void*)) {return sqlite3_bind_text16(a1, a2, a3, a4, a5);}
    static void result_blob(context* a1, const void* a2, int a3, void(*a4)(void*)) {sqlite3_result_blob(a1, a2, a3, a4);}
    static void result_double(context* a1, double a2) {sqlite3_result_double(a1, a2);}
    static void result_error16(context* a1, const void* a2, int a3) {sqlite3_result_error16(a1, a2, a3);}
    static void result_int(context* a1, int a2) {sqlite3_result_int(a1, a2);}
    static void result_int64(context* a1, int64 a2) {sqlite3_result_int64(a1, a2);}
    static void result_null(context* a1) {sqlite3_result_null(a1);}
    static void result_text16(context* a1, const void* a2, int a3, void(*a4)(void*)) {sqlite3_result_text16(a1, a2, a3, a4);}

    // Override open_v2 to use Adiantum VFS
    static int open_v2(const char* filename, handle** ppDb, int flags, const char* zVfs);

    static int finalize(stmt* arg) {return sqlite3_finalize(arg);}
    static const char* errmsg(handle* arg) {return sqlite3_errmsg(arg);}
    static const char* column_decltype(stmt* arg1, int arg2) {return sqlite3_column_decltype(arg1, arg2);}
    static int column_type(stmt* arg1, int arg2) {return sqlite3_column_type(arg1, arg2);}
    static int extended_errcode(handle* arg) {return sqlite3_extended_errcode(arg);}
    static const void* column_blob(stmt* arg1, int arg2) {return sqlite3_column_blob(arg1, arg2);}
    static int column_bytes(stmt* arg1, int arg2) {return sqlite3_column_bytes(arg1, arg2);}
    static int column_bytes16(stmt* arg1, int arg2) {return sqlite3_column_bytes16(arg1, arg2);}
    static double column_double(stmt* arg1, int arg2) {return sqlite3_column_double(arg1, arg2);}
    static int64 column_int64(stmt* arg1, int arg2) {return sqlite3_column_int64(arg1, arg2);}
    static const void* column_text16(stmt* arg1, int arg2) {return sqlite3_column_text16(arg1, arg2);}
    static const char* column_name(stmt* arg1, int arg2) {return sqlite3_column_name(arg1, arg2);}
    static int column_count(stmt* arg1) {return sqlite3_column_count(arg1);}
    static const char* column_database_name(stmt* arg1, int arg2) {return sqlite3_column_database_name(arg1, arg2);}
    static const char* column_table_name(stmt* arg1, int arg2) {return sqlite3_column_table_name(arg1, arg2);}
    static const char* column_origin_name(stmt* arg1, int arg2) {return sqlite3_column_origin_name(arg1, arg2);}
    static int changes(handle* arg) {return sqlite3_changes(arg);}
    static int64 changes64(handle* arg) {return sqlite3_changes64(arg);}
    static int total_changes(handle* arg) {return sqlite3_total_changes(arg);}
    static int64 total_changes64(handle* arg) {return sqlite3_total_changes64(arg);}
    static int64 last_insert_rowid(handle* arg) {return sqlite3_last_insert_rowid(arg);}
    static int step(stmt* arg) {return sqlite3_step(arg);}
    static int reset(stmt* arg) {return sqlite3_reset(arg);}
    static int close(handle* arg) {return sqlite3_close(arg);}
    static void free(void* arg) {return sqlite3_free(arg);}
    static int threadsafe() {return sqlite3_threadsafe();}
    static const char* libversion() {return sqlite3_libversion();}
    static int libversion_number() {return sqlite3_libversion_number();}
    template<typename... Args>
    static int db_config(handle* arg1, int arg2, Args&&... args) {return sqlite3_db_config(arg1, arg2, std::forward<Args>(args)...);}
    static int get_autocommit(handle* arg) {return sqlite3_get_autocommit(arg);}
    static int wal_checkpoint(handle* arg1, const char* arg2) {return sqlite3_wal_checkpoint(arg1, arg2);}
    static int wal_checkpoint_v2(handle* a1, const char* a2, int a3, int* a4, int* a5) {return sqlite3_wal_checkpoint_v2(a1, a2, a3, a4, a5);}
    static int load_extension(handle* arg1, const char* arg2, const char* arg3, char** arg4) {return sqlite3_load_extension(arg1, arg2, arg3, arg4);}
    static void* user_data(context* arg) {return sqlite3_user_data(arg);}
    static void* aggregate_context(context* arg1, int arg2) {return sqlite3_aggregate_context(arg1, arg2);}
    static int collation_needed(handle* a1, void* a2, void(*a3)(void*,handle*,int eTextRep,const char*)) {return sqlite3_collation_needed(a1, a2, a3);}
    static int prepare_v2(handle* a1, const char* a2, int a3, stmt** a4, const char** a5) {return sqlite3_prepare_v2(a1, a2, a3, a4, a5);}
    static int create_function(handle* a1, const char* a2, int a3, int a4, void* a5, void (*a6)(context*,int,value**), void (*a7)(context*,int,value**), void (*a8)(context*)) \
        {return sqlite3_create_function(a1, a2, a3, a4, a5, a6, a7, a8);}
    static int create_function_v2(handle* a1, const char* a2, int a3, int a4, void* a5, void (*a6)(context*,int,value**), void (*a7)(context*,int,value**), void (*a8)(context*), void(*a9)(void*)) \
        {return sqlite3_create_function_v2(a1, a2, a3, a4, a5, a6, a7, a8, a9);}
    static int create_window_function(handle* a1, const char* a2, int a3, int a4, void* a5, void (*a6)(context*,int,value**), void (*a7)(context*), void (*a8)(context*), void (*a9)(context*,int,value**), void(*a10)(void*)) \
        {return sqlite3_create_window_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);}
    static int create_collation_v2(handle* a1, const char* a2, int a3, void* a4, int(*a5)(void*,int,const void*,int,const void*), void(*a6)(void*)) \
        {return sqlite3_create_collation_v2(a1, a2, a3, a4, a5, a6);}
    static int complete(const char* arg) {return sqlite3_complete(arg);}
};

class DbAdiantumInstance : public AbstractDb3<AdiantumDriver>
{
    public:
        DbAdiantumInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

        Db* clone() const;
        QString getTypeClassName() const;
        QString getTypeLabel() const;

    protected:
        bool openInternal();
        void initAfterOpen();
        QString getAttachSql(Db* otherDb, const QString& generatedAttachName);
};

#endif // DBADIANTUMINSTANCE_H
