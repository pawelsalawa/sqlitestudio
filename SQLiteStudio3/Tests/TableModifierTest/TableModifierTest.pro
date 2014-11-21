#-------------------------------------------------
#
# Project created by QtCreator 2013-11-23T15:08:59
#
#-------------------------------------------------

include($$PWD/../../dirs.pri)
include($$PWD/../testdirs.pri)

QT       += testlib
QT       -= gui

TARGET = tst_tablemodifiertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_tablemodifiertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
