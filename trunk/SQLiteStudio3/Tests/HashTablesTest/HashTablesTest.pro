#-------------------------------------------------
#
# Project created by QtCreator 2014-02-12T16:34:55
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)
include($$PWD/../testdirs.pri)

QT       += testlib

QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

TARGET = tst_hashtablestesttest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_hashtablestesttest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -lTestUtils -lcoreSQLiteStudio
