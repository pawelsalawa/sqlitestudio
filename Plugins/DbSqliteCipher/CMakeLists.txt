find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    dbsqlitecipher_global.h
    dbsqlitecipher.h
    dbsqlitecipherinstance.h
    sqlcipher.h
)

set(PROJECT_SOURCES
    dbsqlitecipher.cpp
    dbsqlitecipherinstance.cpp
)
qt_add_resources(PROJECT_SOURCES
    dbsqlitecipher.qrc
)

add_library(DbSqliteCipher SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(DbSqliteCipher)
target_compile_definitions(DbSqliteCipher PRIVATE
    DBSQLITECIPHER_LIBRARY
)
target_link_libraries(DbSqliteCipher PRIVATE
    Qt::Core
)
