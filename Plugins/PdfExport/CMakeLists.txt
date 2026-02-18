find_package(Qt6 REQUIRED COMPONENTS Core Gui)

set(PROJECT_HEADERS
    pdfexport_global.h
    pdfexport.h
)

set(PROJECT_SOURCES
    pdfexport.cpp
)
qt_add_resources(PROJECT_SOURCES
    pdfexport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    pdfexport.ui
)

add_library(PdfExport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(PdfExport)
target_compile_definitions(PdfExport PRIVATE
    PDFEXPORT_LIBRARY
)
target_link_libraries(PdfExport PRIVATE
    Qt::Core
    Qt::Gui
)
