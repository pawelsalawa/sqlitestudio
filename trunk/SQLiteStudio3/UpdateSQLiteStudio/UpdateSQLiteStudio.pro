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
#CONFIG   += console
CONFIG   -= app_bundle

CONFIG   += c++11
QMAKE_CXXFLAGS += -pedantic

LIBS += -lcoreSQLiteStudio

TEMPLATE = app

linux|portable {
    QMAKE_LFLAGS += -Wl,-rpath,./lib
}


win32: {
    RC_FILE = windows.rc
}

SOURCES += main.cpp

OTHER_FILES += \
    windows.rc \
    UpdateSQLiteStudio.exe.manifest

HEADERS +=
