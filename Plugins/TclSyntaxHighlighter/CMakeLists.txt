find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(PROJECT_HEADERS
    TclSyntaxHighlighter_global.h
    tclsyntaxhighlighter.h
)

set(PROJECT_SOURCES
    tclsyntaxhighlighter.cpp
)

add_library(TclSyntaxHighlighter SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(TclSyntaxHighlighter)
target_compile_definitions(TclSyntaxHighlighter PRIVATE
    TCLSYNTAXHIGHLIGHTER_LIBRARY
)
target_link_libraries(TclSyntaxHighlighter PRIVATE
    Qt::Core
    Qt::Widgets
)
