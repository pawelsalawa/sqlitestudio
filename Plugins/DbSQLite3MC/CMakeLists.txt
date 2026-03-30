option(WITH_SYSTEM_SQLITE_MC "Use system SQLiteMultipleCiphers" OFF)

find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    dbsqlitemc_global.h
    dbsqlitemc.h
    dbsqlitemcinstance.h
)

set(PROJECT_SOURCES
    dbsqlitemc.cpp
    dbsqlitemcinstance.cpp
)

if(NOT WITH_SYSTEM_SQLITE_MC)
    list(APPEND PROJECT_HEADERS sqlite3mc_amalgamation.h)
    list(APPEND PROJECT_SOURCES sqlite3mc_amalgamation.c)
endif()

add_library(DbSqliteMc SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(DbSqliteMc)
target_compile_definitions(DbSqliteMc PRIVATE
    DBSQLITEMC_LIBRARY
)
target_link_libraries(DbSqliteMc PRIVATE
    Qt::Core
)

if(WITH_SYSTEM_SQLITE_MC)
    target_compile_definitions(DbSqliteMc PRIVATE
        SQLITE3MC_SYSTEM_LIB
    )
    target_link_libraries(DbSqliteMc PRIVATE
        sqlite3mc
    )
else()
    if(WIN32)
        target_compile_definitions(DbSqliteMc PRIVATE
            SQLITE_OS_WIN=1
            DISABLE_AVX512
            DISABLE_AVX2
        )
    else()
        target_compile_definitions(DbSqliteMc PRIVATE
            SQLITE_OS_UNIX=1
        )
    endif()

    target_compile_options(DbSqliteMc PRIVATE
        -Wall
        -Wno-unused-parameter
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-but-set-variable
        -Wno-parentheses
        -Wno-unused-variable
        -Wno-unknown-pragmas
    )
    target_compile_definitions(DbSqliteMc PRIVATE
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
