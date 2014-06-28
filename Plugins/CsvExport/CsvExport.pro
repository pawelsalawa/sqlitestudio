#-------------------------------------------------
#
# Project created by QtCreator 2014-06-14T16:04:07
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT       -= gui

TARGET = CsvExport
TEMPLATE = lib

DEFINES += CSVEXPORT_LIBRARY

SOURCES += csvexport.cpp

HEADERS += csvexport.h\
        csvexport_global.h

FORMS += \
    CsvExport.ui

OTHER_FILES += \
    csvexport.json

RESOURCES += \
    csvexport.qrc

