#-------------------------------------------------
#
# Project created by QtCreator 2013-04-08T22:41:09
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = DbSqlite2
TEMPLATE = lib

DEFINES += DBSQLITE2_LIBRARY

SOURCES += dbsqlite2.cpp \
    dbsqlite2instance.cpp

HEADERS += dbsqlite2.h\
        dbsqlite2_global.h \
    dbsqlite2instance.h

LIBS += -lsqlite

OTHER_FILES += \
    dbsqlite2.json












