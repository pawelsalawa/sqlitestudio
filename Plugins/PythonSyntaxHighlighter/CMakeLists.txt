find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(PROJECT_HEADERS
    pythonsyntaxhighlighter_global.h
    pythonsyntaxhighlighter.h
)

set(PROJECT_SOURCES
    pythonsyntaxhighlighter.cpp
)

add_library(PythonSyntaxHighlighter SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(PythonSyntaxHighlighter)
target_compile_definitions(PythonSyntaxHighlighter PRIVATE
    PYTHONSYNTAXHIGHLIGHTER_LIBRARY
)
target_link_libraries(PythonSyntaxHighlighter PRIVATE
    Qt::Core
    Qt::Widgets
)
