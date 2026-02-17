find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_HEADERS
    jsonhighlighter.h
    multieditorjson_global.h
    multieditorjson.h
)

set(PROJECT_SOURCES
    jsonhighlighter.cpp
    multieditorjson.cpp
)
qt_add_resources(PROJECT_SOURCES
    multieditorjson.qrc
)

add_library(MultiEditorJson SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(MultiEditorJson)
target_compile_definitions(MultiEditorJson PRIVATE
    MULTIEDITORJSON_LIBRARY
)
target_link_libraries(MultiEditorJson PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
