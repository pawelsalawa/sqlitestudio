find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    jsonexport_global.h
    jsonexport.h
)

set(PROJECT_SOURCES
    jsonexport.cpp
)
qt_add_resources(PROJECT_SOURCES
    jsonexport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    jsonexport.ui
)

add_library(JsonExport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(JsonExport)
target_compile_definitions(JsonExport PRIVATE
    JSONEXPORT_LIBRARY
)
target_link_libraries(JsonExport PRIVATE
    Qt::Core
)
