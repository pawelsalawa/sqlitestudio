find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    htmlexport_global.h
    htmlexport.h
)

set(PROJECT_SOURCES
    htmlexport.cpp
)
qt_add_resources(PROJECT_SOURCES
    htmlexport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    htmlexport.ui
)

add_library(HtmlExport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(HtmlExport)
target_compile_definitions(HtmlExport PRIVATE
    HTMLEXPORT_LIBRARY
)
target_link_libraries(HtmlExport PRIVATE
    Qt::Core
)
