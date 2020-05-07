#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T18:21:00
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = SqlExport
TEMPLATE = lib

DEFINES += SQLEXPORT_LIBRARY

SOURCES += sqlexport.cpp

HEADERS += sqlexport.h\
        sqlexport_global.h

FORMS += \
    SqlExportQuery.ui \
    SqlExportCommon.ui

OTHER_FILES += \
    sqlexport.json

RESOURCES += \
    sqlexport.qrc


TRANSLATIONS += translations/SqlExport.ts \
		translations/SqlExport_ro_RO.ts \
		translations/SqlExport_de_DE.ts \
		translations/SqlExport_it_IT.ts \
		translations/SqlExport_zh_CN.ts \
		translations/SqlExport_sk_SK.ts \
		translations/SqlExport_ru_RU.ts \
		translations/SqlExport_pt_BR.ts \
		translations/SqlExport_fr_FR.ts \
		translations/SqlExport_es_ES.ts \
		translations/SqlExport_pl_PL.ts



























