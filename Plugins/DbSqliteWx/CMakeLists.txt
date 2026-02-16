option(WITH_SYSTEM_WXSQLITE "Use system wxSQLite" OFF)

find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    dbsqlitewx_global.h
    dbsqlitewx.h
    dbsqlitewxinstance.h
)

set(PROJECT_SOURCES
    dbsqlitewx.cpp
    dbsqlitewxinstance.cpp
)

if(NOT WITH_SYSTEM_WXSQLITE)
    list(APPEND PROJECT_HEADERS wxsqlite3.h)
    list(APPEND PROJECT_SOURCES wxsqlite3.c)
endif()

add_library(DbSqliteWx SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(DbSqliteWx)
target_compile_definitions(DbSqliteWx PRIVATE
    DBSQLITEWX_LIBRARY
)
target_link_libraries(DbSqliteWx PRIVATE
    Qt::Core
)

if(WITH_SYSTEM_WXSQLITE)
    target_compile_definitions(DbSqliteWx PRIVATE
        WXSQLITE_SYSTEM_LIB
    )
    target_link_libraries(DbSqliteWx PRIVATE
        wxsqlite3
    )
else()
    if(WIN32)
        target_compile_definitions(DbSqliteWx PRIVATE
            SQLITE_OS_WIN=1
        )
    else()
        target_compile_definitions(DbSqliteWx PRIVATE
            SQLITE_OS_UNIX=1
        )
    endif()

    target_compile_options(DbSqliteWx PRIVATE
        -Wall
        -Wno-unused-parameter
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-but-set-variable
        -Wno-parentheses
        -Wno-unused-variable
        -Wno-unknown-pragmas
    )
    target_compile_definitions(DbSqliteWx PRIVATE
        SQLITE_HAS_CODEC
        SQLITE_ALLOW_XTHREAD_CONNECT=1
        SQLITE_THREADSAFE=1
        SQLITE_TEMP_STORE=2
        CODEC_TYPE=CODEC_TYPE_AES256
        SQLITE_CORE
        USE_DYNAMIC_SQLITE3_LOAD=0
        SQLITE_ENABLE_UPDATE_DELETE_LIMIT=1
        SQLITE_ENABLE_DBSTAT_VTAB=1
        SQLITE_ENABLE_BYTECODE_VTAB=1
        SQLITE_ENABLE_COLUMN_METADATA=1
        SQLITE_ENABLE_EXPLAIN_COMMENTS=1
        SQLITE_ENABLE_FTS3=1
        SQLITE_ENABLE_FTS3_PARENTHESIS=1
        SQLITE_ENABLE_FTS4=1
        SQLITE_ENABLE_FTS5=1
        SQLITE_ENABLE_GEOPOLY=1
        SQLITE_ENABLE_JSON1=1
        SQLITE_ENABLE_RTREE=1
        SQLITE_ENABLE_MATH_FUNCTIONS=1
    )
endif()
