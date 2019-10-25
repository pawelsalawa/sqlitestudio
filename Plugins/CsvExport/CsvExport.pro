#-------------------------------------------------
#
# Project created by QtCreator 2014-06-14T16:04:07
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = CsvExport
TEMPLATE = lib

DEFINES += CSVEXPORT_LIBRARY

SOURCES += csvexport.cpp

HEADERS += csvexport.h\
        csvexport_global.h

FORMS += \
    CsvExport.ui

OTHER_FILES += \
    csvexport.json

RESOURCES += \
    csvexport.qrc



TRANSLATIONS += translations/CsvExport_ro_RO.ts \
		translations/CsvExport_de_DE.ts \
		translations/CsvExport_it_IT.ts \
		translations/CsvExport_zh_CN.ts \
		translations/CsvExport_sk_SK.ts \
		translations/CsvExport_ru_RU.ts \
		translations/CsvExport_pt_BR.ts \
		translations/CsvExport_fr_FR.ts \
		translations/CsvExport_es_ES.ts \
		translations/CsvExport_pl_PL.ts


























