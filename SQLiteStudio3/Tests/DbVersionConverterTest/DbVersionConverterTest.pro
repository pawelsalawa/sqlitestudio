#-------------------------------------------------
#
# Project created by QtCreator 2014-05-04T00:44:01
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)
include($$PWD/../testdirs.pri)

QT       += testlib

QT       -= gui

TARGET = tst_dbversionconvertertesttest
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app


SOURCES += tst_dbversionconvertertesttest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -lTestUtils -lcoreSQLiteStudio
