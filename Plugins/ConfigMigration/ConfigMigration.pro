#-------------------------------------------------
#
# Project created by QtCreator 2014-09-15T15:02:05
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TARGET = ConfigMigration
TEMPLATE = lib

DEFINES += CONFIGMIGRATION_LIBRARY

SOURCES += configmigration.cpp

HEADERS += configmigration.h\
        configmigration_global.h

OTHER_FILES += \
    configmigration.json
