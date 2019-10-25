#-------------------------------------------------
#
# Project created by QtCreator 2014-05-17T20:38:58
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = CsvImport
TEMPLATE = lib

DEFINES += CSVIMPORT_LIBRARY

SOURCES += csvimport.cpp

HEADERS += csvimport.h\
        csvimport_global.h

FORMS += \
    CsvImportOptions.ui

OTHER_FILES += \
    csvimport.json

RESOURCES += \
    csvimport.qrc


TRANSLATIONS += translations/CsvImport_ro_RO.ts \
		translations/CsvImport_de_DE.ts \
		translations/CsvImport_it_IT.ts \
		translations/CsvImport_zh_CN.ts \
		translations/CsvImport_sk_SK.ts \
		translations/CsvImport_ru_RU.ts \
		translations/CsvImport_pt_BR.ts \
		translations/CsvImport_fr_FR.ts \
		translations/CsvImport_es_ES.ts \
		translations/CsvImport_pl_PL.ts


























