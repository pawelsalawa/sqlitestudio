#-------------------------------------------------
#
# Project created by QtCreator 2013-05-30T16:51:11
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)
include($$PWD/../testdirs.pri)

QT       += sql testlib

QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

TARGET = tst_parsertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_parsertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -lTestUtils -lcoreSQLiteStudio
