#-------------------------------------------------
#
# Project created by QtCreator 2013-03-29T23:25:17
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)

QT       += sql
QT       -= gui

TARGET = TestUtils
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11

DEFINES += TESTUTILS_LIBRARY

SOURCES += \
    dbsqlite3mock.cpp

HEADERS +=\
        testutils_global.h \
    dbsqlite3mock.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

LIBS += -lcoreSQLiteStudio
