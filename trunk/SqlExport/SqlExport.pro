#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T18:21:00
#
#-------------------------------------------------

include($$PWD/../SQLiteStudio3/plugins.pri)
include($$PWD/../SQLiteStudio3/utils.pri)

QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

TARGET = SqlExport
TEMPLATE = lib

DEFINES += SQLEXPORT_LIBRARY

SOURCES += sqlexport.cpp

HEADERS += sqlexport.h\
        sqlexport_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    SqlExportQuery.ui

copy_file(SqlExportQuery.ui, $$DESTDIR/SqlExportQuery.ui)
