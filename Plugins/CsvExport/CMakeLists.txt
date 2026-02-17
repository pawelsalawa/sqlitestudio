find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    csvexport_global.h
    csvexport.h
)

set(PROJECT_SOURCES
    csvexport.cpp
)
qt_add_resources(PROJECT_SOURCES
    csvexport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    CsvExport.ui
)

add_library(CsvExport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(CsvExport)
target_compile_definitions(CsvExport PRIVATE
    CSVEXPORT_LIBRARY
)
target_link_libraries(CsvExport PRIVATE
    Qt::Core
)
