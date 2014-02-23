#-------------------------------------------------
#
# Project created by QtCreator 2014-02-18T20:17:53
#
#-------------------------------------------------

include($$PWD/../SQLiteStudio3/plugins.pri)
include($$PWD/../SQLiteStudio3/utils.pri)

QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

TARGET = SqlFunctionSql
TEMPLATE = lib

DEFINES += SQLFUNCTIONSQL_LIBRARY

SOURCES += sqlfunctionsql.cpp

HEADERS += sqlfunctionsql.h\
        sqlfunctionsql_global.h
