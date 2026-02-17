find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    csvimport_global.h
    csvimport.h
)

set(PROJECT_SOURCES
    csvimport.cpp
)
qt_add_resources(PROJECT_SOURCES
    csvimport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    CsvImportOptions.ui
)

add_library(CsvImport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(CsvImport)
target_compile_definitions(CsvImport PRIVATE
    CSVIMPORT_LIBRARY
)
target_link_libraries(CsvImport PRIVATE
    Qt::Core
)
