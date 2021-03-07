include($$PWD/../TestUtils/test_common.pri)

QT       += testlib
QT       -= gui

TARGET = tst_formattertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_formattertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
