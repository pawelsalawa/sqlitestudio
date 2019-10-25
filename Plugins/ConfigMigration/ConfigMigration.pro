#-------------------------------------------------
#
# Project created by QtCreator 2014-09-15T15:02:05
#
#-------------------------------------------------

QT += widgets

include($$PWD/../../SQLiteStudio3/plugins.pri)

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


TRANSLATIONS += translations/ConfigMigration.ts \
		translations/ConfigMigration_ro_RO.ts \
		translations/ConfigMigration_de_DE.ts \
		translations/ConfigMigration_it_IT.ts \
		translations/ConfigMigration_zh_CN.ts \
		translations/ConfigMigration_sk_SK.ts \
		translations/ConfigMigration_ru_RU.ts \
		translations/ConfigMigration_pt_BR.ts \
		translations/ConfigMigration_fr_FR.ts \
		translations/ConfigMigration_es_ES.ts \
		translations/ConfigMigration_pl_PL.ts



























