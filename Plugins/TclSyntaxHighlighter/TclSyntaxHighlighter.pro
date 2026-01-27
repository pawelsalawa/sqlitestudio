include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TEMPLATE = lib
DEFINES += TCLSYNTAXHIGHLIGHTER_LIBRARY

SOURCES += \
    tclsyntaxhighlighter.cpp

HEADERS += \
    TclSyntaxHighlighter_global.h \
    tclsyntaxhighlighter.h


OTHER_FILES += \
    tclsyntaxhighlighter.json
