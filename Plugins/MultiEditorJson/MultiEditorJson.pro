include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TARGET = MultiEditorJson
TEMPLATE = lib

DEFINES += MULTIEDITORJSON_LIBRARY

SOURCES += \
    multieditorjson.cpp \
    jsonhighlighter.cpp

HEADERS += \
    multieditorjson.h \
    multieditorjson_global.h \
    jsonhighlighter.h

OTHER_FILES += \
    multieditorjson.json

RESOURCES += \
    multieditorjson.qrc
