find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_HEADERS
    multieditorimage_global.h
    multieditorimage.h
)

set(PROJECT_SOURCES
    multieditorimage.cpp
)
qt_add_resources(PROJECT_SOURCES
    multieditorimage.qrc
)

add_library(MultiEditorImage SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(MultiEditorImage)
target_compile_definitions(MultiEditorImage PRIVATE
    MULTIEDITORIMAGE_LIBRARY
)
target_link_libraries(MultiEditorImage PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
