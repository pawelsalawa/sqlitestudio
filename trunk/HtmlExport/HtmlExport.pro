#-------------------------------------------------
#
# Project created by QtCreator 2014-06-20T22:57:37
#
#-------------------------------------------------

include($$PWD/../SQLiteStudio3/plugins.pri)
include($$PWD/../SQLiteStudio3/utils.pri)

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += $$PWD/../DocExport
win32: {
    LIBS += -lDocExport
}

TARGET = HtmlExport
TEMPLATE = lib

DEFINES += HTMLEXPORT_LIBRARY

SOURCES += htmlexport.cpp

HEADERS += htmlexport.h\
        htmlexport_global.h

OTHER_FILES += \
    htmlexport.json
