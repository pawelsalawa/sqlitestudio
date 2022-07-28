#-------------------------------------------------
#
# Project created by QtCreator 2014-07-20T12:19:38
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = RegExpImport
TEMPLATE = lib

DEFINES += REGEXPIMPORT_LIBRARY

SOURCES += regexpimport.cpp

HEADERS += regexpimport.h\
        regexpimport_global.h

OTHER_FILES += \
    regexpimport.json

FORMS += \
    regexpimport.ui

RESOURCES += \
    regexpimport.qrc
