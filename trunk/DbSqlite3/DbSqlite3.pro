#-------------------------------------------------
#
# Project created by QtCreator 2013-04-08T22:09:17
#
#-------------------------------------------------

include($$PWD/../SQLiteStudio3/plugins.pri)

QT       -= gui
QT       += sql

QMAKE_CXXFLAGS += -std=c++11

TARGET = DbSqlite3
TEMPLATE = lib

DEFINES += DBSQLITE3_LIBRARY

SOURCES += dbsqlite3.cpp \
    dbsqlite3instance.cpp

HEADERS += dbsqlite3.h \
        dbsqlite3_global.h \
    dbsqlite3instance.h

LIBS += -lsqlite3

OTHER_FILES += \
    building_qt_plugin.txt
