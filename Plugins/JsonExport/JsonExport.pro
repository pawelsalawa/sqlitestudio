#-------------------------------------------------
#
# Project created by QtCreator 2014-07-16T20:04:29
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = JsonExport
TEMPLATE = lib

DEFINES += JSONEXPORT_LIBRARY

SOURCES += jsonexport.cpp

HEADERS += jsonexport.h\
        jsonexport_global.h

OTHER_FILES += \
    jsonexport.json

FORMS += \
    jsonexport.ui

RESOURCES += \
    jsonexport.qrc


TRANSLATIONS += translations/JsonExport.ts \
		translations/JsonExport_ro_RO.ts \
		translations/JsonExport_de_DE.ts \
		translations/JsonExport_it_IT.ts \
		translations/JsonExport_zh_CN.ts \
		translations/JsonExport_sk_SK.ts \
		translations/JsonExport_ru_RU.ts \
		translations/JsonExport_pt_BR.ts \
		translations/JsonExport_fr_FR.ts \
		translations/JsonExport_es_ES.ts \
		translations/JsonExport_pl_PL.ts



























