#-------------------------------------------------
#
# Project created by QtCreator 2014-08-29T20:30:14
#
#-------------------------------------------------

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

QT       += core
QT       -= gui

TARGET = UpdateSQLiteStudio
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

win32: {
    RC_FILE = windows.rc
}

SOURCES += main.cpp

OTHER_FILES += \
    windows.rc \
    UpdateSQLiteStudio.exe.manifest
