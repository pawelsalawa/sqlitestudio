#-------------------------------------------------
#
# Project created by QtCreator 2014-07-20T12:19:38
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = RegExpImport
TEMPLATE = lib

DEFINES += REGEXPIMPORT_LIBRARY

SOURCES += regexpimport.cpp

HEADERS += regexpimport.h\
        regexpimport_global.h

OTHER_FILES += \
    regexpimport.json

FORMS += \
    regexpimport.ui

RESOURCES += \
    regexpimport.qrc


TRANSLATIONS += translations/RegExpImport_ro_RO.ts \
		translations/RegExpImport_de_DE.ts \
		translations/RegExpImport_it_IT.ts \
		translations/RegExpImport_zh_CN.ts \
		translations/RegExpImport_sk_SK.ts \
		translations/RegExpImport_ru_RU.ts \
		translations/RegExpImport_pt_BR.ts \
		translations/RegExpImport_fr_FR.ts \
		translations/RegExpImport_es_ES.ts \
		translations/RegExpImport_pl_PL.ts


























