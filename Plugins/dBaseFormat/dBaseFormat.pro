#-------------------------------------------------
#
# Project created by QtCreator 2014-07-18T21:14:40
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT       -= gui

TARGET = dBaseFormat
TEMPLATE = lib

DEFINES += DBASEFORMAT_LIBRARY

SOURCES += dbaseformat.cpp \
    qdbf/qdbffield.cpp \
    qdbf/qdbfrecord.cpp \
    qdbf/qdbftable.cpp \
    qdbf/qdbftablemodel.cpp

HEADERS += dbaseformat.h\
        dbaseformat_global.h \
    qdbf/qdbffield.h \
    qdbf/qdbfrecord.h \
    qdbf/qdbftable.h \
    qdbf/qdbftablemodel.h \
    qdbf/qdbf_global.h

OTHER_FILES += \
    dbaseformat.json \
    qdbf/lgpl.txt
