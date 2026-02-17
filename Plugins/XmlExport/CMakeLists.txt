find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    xmlexport.h
    xmlexport_global.h
)

set(PROJECT_SOURCES
    xmlexport.cpp
)
qt_add_resources(PROJECT_SOURCES
    xmlexport.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    XmlExport.ui
)

add_library(XmlExport SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(XmlExport)
target_compile_definitions(XmlExport PRIVATE
    XMLEXPORT_LIBRARY
)
target_link_libraries(XmlExport PRIVATE
    Qt::Core
)
