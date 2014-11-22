#-------------------------------------------------
#
# Project created by QtCreator 2013-03-29T21:35:35
#
#-------------------------------------------------

include($$PWD/../TestUtils/test_common.pri)

QT       += testlib
QT       -= gui

TARGET = tst_completionhelpertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_completionhelpertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS +=
