#-------------------------------------------------
#
# Project created by QtCreator 2014-06-14T16:04:07
#
#-------------------------------------------------

include($$PWD/../SQLiteStudio3/plugins.pri)
include($$PWD/../SQLiteStudio3/utils.pri)

QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

TARGET = CsvExport
TEMPLATE = lib

DEFINES += CSVEXPORT_LIBRARY

SOURCES += csvexport.cpp

HEADERS += csvexport.h\
        csvexport_global.h

FORMS += \
    CsvExport.ui

copy_file(CsvExport.ui, $$DESTDIR/CsvExport.ui)
