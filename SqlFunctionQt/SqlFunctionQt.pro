#-------------------------------------------------
#
# Project created by QtCreator 2014-03-09T19:31:14
#
#-------------------------------------------------

include($$PWD/../SQLiteStudio3/plugins.pri)
include($$PWD/../SQLiteStudio3/utils.pri)

QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

TARGET = SqlFunctionQt
TEMPLATE = lib

DEFINES += SQLFUNCTIONQT_LIBRARY

SOURCES += sqlfunctionqt.cpp

HEADERS += sqlfunctionqt.h\
        sqlfunctionqt_global.h
