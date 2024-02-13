#-------------------------------------------------
#
# Project created by QtCreator 2023-02-08T20:25:45
#
#-------------------------------------------------

QT += widgets

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = LlmChat
TEMPLATE = lib

DEFINES += LLMCHAT_LIBRARY

SOURCES += llmchat.cpp

HEADERS += llmchat.h

OTHER_FILES += \
    llmchat.json

INCLUDEPATH += $$PLUGINSDIR/LlmChat
DEPENDPATH += $$PLUGINSDIR/LlmChat