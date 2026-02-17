find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    sqlformattersimple_global.h
    sqlformattersimpleplugin.h
)

set(PROJECT_SOURCES
    sqlformattersimpleplugin.cpp
)
qt_add_resources(PROJECT_SOURCES
    sqlformattersimple.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    sqlformattersimple.ui
)

add_library(SqlFormatterSimple SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(SqlFormatterSimple)
target_compile_definitions(SqlFormatterSimple PRIVATE
    SQLFORMATTERSIMPLE_LIBRARY
)
target_link_libraries(SqlFormatterSimple PRIVATE
    Qt::Core
)
