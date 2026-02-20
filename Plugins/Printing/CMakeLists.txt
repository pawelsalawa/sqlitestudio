find_package(Qt6 REQUIRED COMPONENTS Core PrintSupport Widgets)

set(PROJECT_HEADERS
    printing_global.h
    printing.h
    printingexport.h
)

set(PROJECT_SOURCES
    printing.cpp
    printingexport.cpp
)
qt_add_resources(PROJECT_SOURCES
    printing.qrc
)

add_library(Printing SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(Printing)
target_compile_definitions(Printing PRIVATE
    PRINTING_LIBRARY
)
if(WIN32)
	target_link_directories(Printing PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../PdfExport")
	target_link_libraries(Printing PRIVATE PdfExport)
endif()

target_link_libraries(Printing PRIVATE
    Qt::Core
    Qt::Gui
    Qt::PrintSupport
    Qt::Widgets
)
