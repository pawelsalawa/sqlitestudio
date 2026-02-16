find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    regexpimport_global.h
    regexpimport.h
)

set(PROJECT_SOURCES
    regexpimport.cpp
)
qt_add_resources(PROJECT_SOURCES
    regexpimport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    regexpimport.ui
)

add_library(RegExpImport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(RegExpImport)
target_compile_definitions(RegExpImport PRIVATE
    REGEXPIMPORT_LIBRARY
)
target_link_libraries(RegExpImport PRIVATE
    Qt::Core
)
