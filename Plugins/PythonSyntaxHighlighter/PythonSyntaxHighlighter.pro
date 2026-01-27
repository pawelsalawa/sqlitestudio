include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TARGET = PythonSyntaxHighlighter
TEMPLATE = lib

DEFINES += PYTHONSYNTAXHIGHLIGHTER_LIBRARY

SOURCES += pythonsyntaxhighlighter.cpp

HEADERS += pythonsyntaxhighlighter.h\
        pythonsyntaxhighlighter_global.h

OTHER_FILES += \
    pythonsyntaxhighlighter.json
