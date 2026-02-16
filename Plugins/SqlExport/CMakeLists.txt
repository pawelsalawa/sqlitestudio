find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    sqlexport_global.h
    sqlexport.h
)

set(PROJECT_SOURCES
    sqlexport.cpp
)
qt_add_resources(PROJECT_SOURCES
    sqlexport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    SqlExportCommon.ui
    SqlExportQuery.ui
)

add_library(SqlExport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(SqlExport)
target_compile_definitions(SqlExport PRIVATE
    SQLEXPORT_LIBRARY
)
target_link_libraries(SqlExport PRIVATE
    Qt::Core
)
