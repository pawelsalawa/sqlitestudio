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

SOURCES += configmigration.cpp \
    configmigrationwizard.cpp

HEADERS += configmigration.h\
        configmigration_global.h \
    configmigrationwizard.h \
    configmigrationitem.h

OTHER_FILES += \
    configmigration.json

FORMS += \
    configmigrationwizard.ui

RESOURCES += \
    configmigration.qrc


TRANSLATIONS += ConfigMigration_pl.ts \
		ConfigMigration_en.ts

