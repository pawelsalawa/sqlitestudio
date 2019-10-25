#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T05:20:57
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = XmlExport
TEMPLATE = lib

DEFINES += XMLEXPORT_LIBRARY

SOURCES += xmlexport.cpp

HEADERS += xmlexport.h\
        xmlexport_global.h

FORMS += XmlExport.ui

OTHER_FILES += \
    xmlexport.json

RESOURCES += \
    xmlexport.qrc


TRANSLATIONS += translations/XmlExport.ts \
		translations/XmlExport_ro_RO.ts \
		translations/XmlExport_de_DE.ts \
		translations/XmlExport_it_IT.ts \
		translations/XmlExport_zh_CN.ts \
		translations/XmlExport_sk_SK.ts \
		translations/XmlExport_ru_RU.ts \
		translations/XmlExport_pt_BR.ts \
		translations/XmlExport_fr_FR.ts \
		translations/XmlExport_es_ES.ts \
		translations/XmlExport_pl_PL.ts



























