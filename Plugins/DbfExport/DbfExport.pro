#-------------------------------------------------
#
# Project created by QtCreator 2014-07-18T19:15:56
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT       -= gui

TARGET = DbfExport
TEMPLATE = lib

DEFINES += DBFEXPORT_LIBRARY

SOURCES += dbfexport.cpp

HEADERS += dbfexport.h\
        dbfexport_global.h

OTHER_FILES += \
    dbfexport.json

INCLUDEPATH += $$PLUGINSDIR/dBaseFormat
DEPENDPATH += $$PLUGINSDIR/dBaseFormat
win32: {
    LIBS += -ldBaseFormat
}
