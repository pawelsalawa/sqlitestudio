#ifndef DBSQLITE3_H
#define DBSQLITE3_H

#include "abstractdb3.h"
#include "common/global.h"
#include <sqlite3.h>

struct Sqlite3
{
    static_char* label = "SQLite 3";

    static const int OK = SQLITE_OK;
    static const int ERROR = SQLITE_ERROR;
    static const int OPEN_READWRITE = SQLITE_OPEN_READWRITE;
    static const int OPEN_CREATE = SQLITE_OPEN_CREATE;
    static const int UTF8 = SQLITE_UTF8;
    static const int INTEGER = SQLITE_INTEGER;
    static const int FLOAT = SQLITE_FLOAT;
    static const int NULL_TYPE = SQLITE_NULL;
    static const int BLOB = SQLITE_BLOB;
    static const int MISUSE = SQLITE_MISUSE;
    static const int BUSY = SQLITE_BUSY;
    static const int ROW = SQLITE_ROW;
    static const int DONE = SQLITE_DONE;

    typedef sqlite3 sqlite3;
    typedef sqlite3_stmt stmt;
    typedef sqlite3_context context;
    typedef sqlite3_value value;
    typedef sqlite3_int64 int64;
    typedef sqlite3_destructor_type destructor_type;

    static destructor_type TRANSIENT() {return SQLITE_TRANSIENT;}
    static void interrupt(sqlite3* arg) {sqlite3_interrupt(arg);}
    static const void *value_blob(value* arg) {return sqlite3_value_blob(arg);}
    static double value_double(value* arg) {return sqlite3_value_double(arg);}
    static int64 value_int64(value* arg) {return sqlite3_value_int64(arg);}
    static const void *value_text16(value* arg) {return sqlite3_value_text16(arg);}
    static int value_bytes(value* arg) {return sqlite3_value_bytes(arg);}
    static int value_bytes16(value* arg) {return sqlite3_value_bytes16(arg);}
    static int value_type(value* arg) {return sqlite3_value_type(arg);}
    static int bind_blob(stmt* a1, int a2, const void* a3, int a4, void(*a5)(void*)) {return sqlite3_bind_blob(a1, a2, a3, a4, a5);}
    static int bind_double(stmt* a1, int a2, double a3) {return sqlite3_bind_double(a1, a2, a3);}
    static int bind_int(stmt* a1, int a2, int a3) {return sqlite3_bind_int(a1, a2, a3);}
    static int bind_int64(stmt* a1, int a2, int64 a3) {return sqlite3_bind_int64(a1, a2, a3);}
    static int bind_null(stmt* a1, int a2) {return sqlite3_bind_null(a1, a2);}
    static int bind_text16(stmt* a1, int a2, const void* a3, int a4, void(*a5)(void*)) {return sqlite3_bind_text16(a1, a2, a3, a4, a5);}
    static void result_blob(context* a1, const void* a2, int a3, void(*a4)(void*)) {sqlite3_result_blob(a1, a2, a3, a4);}
    static void result_double(context* a1, double a2) {sqlite3_result_double(a1, a2);}
    static void result_error16(context* a1, const void* a2, int a3) {sqlite3_result_error16(a1, a2, a3);}
    static void result_int(context* a1, int a2) {sqlite3_result_int(a1, a2);}
    static void result_int64(context* a1, int64 a2) {sqlite3_result_int64(a1, a2);}
    static void result_null(context* a1) {sqlite3_result_null(a1);}
    static void result_text16(context* a1, const void* a2, int a3, void(*a4)(void*)) {sqlite3_result_text16(a1, a2, a3, a4);}
    static int open_v2(const char *a1, sqlite3 **a2, int a3, const char *a4) {return sqlite3_open_v2(a1, a2, a3, a4);}
    static int finalize(stmt *arg) {return sqlite3_finalize(arg);}
    static const char *errmsg(sqlite3* arg) {return sqlite3_errmsg(arg);}
    static int extended_errcode(sqlite3* arg) {return sqlite3_extended_errcode(arg);}
    static const void *column_blob(stmt* arg1, int arg2) {return sqlite3_column_blob(arg1, arg2);}
    static int column_bytes(stmt* arg1, int arg2) {return sqlite3_column_bytes(arg1, arg2);}
    static int column_bytes16(stmt* arg1, int arg2) {return sqlite3_column_bytes16(arg1, arg2);}
    static double column_double(stmt* arg1, int arg2) {return sqlite3_column_double(arg1, arg2);}
    static int64 column_int64(stmt* arg1, int arg2) {return sqlite3_column_int64(arg1, arg2);}
    static const void *column_text16(stmt* arg1, int arg2) {return sqlite3_column_text16(arg1, arg2);}
    static const char *column_name(stmt* arg1, int arg2) {return sqlite3_column_name(arg1, arg2);}
    static int column_type(stmt* arg1, int arg2) {return sqlite3_column_type(arg1, arg2);}
    static int column_count(stmt* arg1) {return sqlite3_column_count(arg1);}
    static int changes(sqlite3* arg) {return sqlite3_changes(arg);}
    static int last_insert_rowid(sqlite3* arg) {return sqlite3_last_insert_rowid(arg);}
    static int step(stmt* arg) {return sqlite3_step(arg);}
    static int reset(stmt* arg) {return sqlite3_reset(arg);}
    static int close(sqlite3* arg) {return sqlite3_close(arg);}
    static int enable_load_extension(sqlite3* arg1, int arg2) {return sqlite3_enable_load_extension(arg1, arg2);}
    static void* user_data(context* arg) {return sqlite3_user_data(arg);}
    static void* aggregate_context(context* arg1, int arg2) {return sqlite3_aggregate_context(arg1, arg2);}
    static int collation_needed(sqlite3* a1, void* a2, void(*a3)(void*,sqlite3*,int eTextRep,const char*)) {return sqlite3_collation_needed(a1, a2, a3);}
    static int prepare_v2(sqlite3 *a1, const char *a2, int a3, stmt **a4, const char **a5) {return sqlite3_prepare_v2(a1, a2, a3, a4, a5);}
    static int create_function(sqlite3 *a1, const char *a2, int a3, int a4, void *a5, void (*a6)(context*,int,value**), void (*a7)(context*,int,value**), void (*a8)(context*))
        {return sqlite3_create_function(a1, a2, a3, a4, a5, a6, a7, a8);}
    static int create_function_v2(sqlite3 *a1, const char *a2, int a3, int a4, void *a5, void (*a6)(context*,int,value**), void (*a7)(context*,int,value**), void (*a8)(context*), void(*a9)(void*))
        {return sqlite3_create_function_v2(a1, a2, a3, a4, a5, a6, a7, a8, a9);}
    static int create_collation_v2(sqlite3* a1, const char *a2, int a3, void *a4, int(*a5)(void*,int,const void*,int,const void*), void(*a6)(void*))
        {return sqlite3_create_collation_v2(a1, a2, a3, a4, a5, a6);}
};

class API_EXPORT DbSqlite3 : public AbstractDb3<Sqlite3>
{
    public:
        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         *
         * All values from this constructor are just passed to AbstractDb3 constructor.
         */
        DbSqlite3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);

        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @overload
         */
        DbSqlite3(const QString& name, const QString& path);
};

#endif // DBSQLITE3_H
