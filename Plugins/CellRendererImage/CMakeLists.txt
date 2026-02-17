find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_HEADERS
    cellrendererimage_global.h
    cellrendererimage.h
    cellrendererimageplugin.h
)

set(PROJECT_SOURCES
    cellrendererimage.cpp
    cellrendererimageplugin.cpp
)
qt_add_resources(PROJECT_SOURCES
    cellrendererimage.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    cellrendererimage.ui
)

add_library(CellRendererImage SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(CellRendererImage)
target_compile_definitions(CellRendererImage PRIVATE
    CELLRENDERERIMAGE_LIBRARY
)
target_link_libraries(CellRendererImage PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
