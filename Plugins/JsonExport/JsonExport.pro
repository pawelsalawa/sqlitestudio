#-------------------------------------------------
#
# Project created by QtCreator 2014-07-16T20:04:29
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = JsonExport
TEMPLATE = lib

DEFINES += JSONEXPORT_LIBRARY

SOURCES += jsonexport.cpp

HEADERS += jsonexport.h\
        jsonexport_global.h

OTHER_FILES += \
    jsonexport.json

FORMS += \
    jsonexport.ui

RESOURCES += \
    jsonexport.qrc
