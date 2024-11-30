#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T18:21:00
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = SqlExport
TEMPLATE = lib

DEFINES += SQLEXPORT_LIBRARY

SOURCES += sqlexport.cpp

HEADERS += sqlexport.h\
        sqlexport_global.h

FORMS += \
    SqlExportQuery.ui \
    SqlExportCommon.ui

OTHER_FILES += \
    sqlexport.json

RESOURCES += \
    sqlexport.qrc
