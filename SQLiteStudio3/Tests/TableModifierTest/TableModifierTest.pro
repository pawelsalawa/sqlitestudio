#-------------------------------------------------
#
# Project created by QtCreator 2013-11-23T15:08:59
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)
include($$PWD/../testdirs.pri)

QT       += sql testlib

QT       -= gui

TARGET = tst_tablemodifiertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += tst_tablemodifiertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -lTestUtils -lcoreSQLiteStudio
