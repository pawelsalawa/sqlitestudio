#-------------------------------------------------
#
# Project created by QtCreator 2014-09-06T00:39:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/sqlitestudio
MOC_DIR = $$MOC_DIR/sqlitestudio
UI_DIR = $$UI_DIR/sqlitestudio

TARGET = sqlitestudio
TEMPLATE = app

CONFIG   += c++11
QMAKE_CXXFLAGS += -pedantic

LIBS += -lcoreSQLiteStudio -lguiSQLiteStudio

SOURCES += main.cpp

win32: {
    RC_FILE = windows.rc
}

OTHER_FILES += \
    windows.rc \
    windows.manifest
