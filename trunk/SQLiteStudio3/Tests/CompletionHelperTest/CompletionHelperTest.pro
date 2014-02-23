#-------------------------------------------------
#
# Project created by QtCreator 2013-03-29T21:35:35
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)
include($$PWD/../testdirs.pri)

QT       += sql testlib

QT       -= gui

TARGET = tst_completionhelpertest
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

SOURCES += tst_completionhelpertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS +=

LIBS += -lTestUtils -lcoreSQLiteStudio
