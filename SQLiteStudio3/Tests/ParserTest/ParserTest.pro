#-------------------------------------------------
#
# Project created by QtCreator 2013-05-30T16:51:11
#
#-------------------------------------------------

include($$PWD/../TestUtils/test_common.pri)

QT       += testlib
QT       -= gui

TARGET = tst_parsertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_parsertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
