#-------------------------------------------------
#
# Project created by QtCreator 2013-02-28T23:21:43
#
#-------------------------------------------------

include($$PWD/../dirs.pri)

OBJECTS_DIR = $$OBJECTS_DIR/Console
MOC_DIR = $$MOC_DIR/Console
UI_DIR = $$UI_DIR/Console

QT       += core sql
QT       -= gui

TARGET = Console
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11 -pedantic

SOURCES += main.cpp \
    cli.cpp \
    commands/clicommand.cpp \
    commands/clicommandfactory.cpp \
    commands/clicommandadd.cpp \
    commands/clicommandremove.cpp \
    commands/clicommandexit.cpp \
    commands/clicommanddblist.cpp \
    commands/clicommanduse.cpp \
    commands/clicommandopen.cpp \
    commands/clicommandclose.cpp \
    commands/clicommandsql.cpp \
    clicommandexecutor.cpp \
    cli_config.cpp \
    commands/clicommandhelp.cpp \
    cliutils.cpp

LIBS += -lcoreSQLiteStudio

win32: {
    INCLUDEPATH += $$PWD/../../../include
    LIBS += -L$$PWD/../../../lib -ledit_static
}

unix: {
    LIBS += -lreadline -ltermcap
}

HEADERS += \
    cli.h \
    commands/clicommand.h \
    commands/clicommandfactory.h \
    commands/clicommandadd.h \
    commands/clicommandremove.h \
    commands/clicommandexit.h \
    commands/clicommanddblist.h \
    commands/clicommanduse.h \
    commands/clicommandopen.h \
    commands/clicommandclose.h \
    commands/clicommandsql.h \
    cli_config.h \
    clicommandexecutor.h \
    commands/clicommandhelp.h \
    cliutils.h
