#-------------------------------------------------
#
# Project created by QtCreator 2014-06-20T22:57:37
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = HtmlExport
TEMPLATE = lib

DEFINES += HTMLEXPORT_LIBRARY

SOURCES += htmlexport.cpp

HEADERS += htmlexport.h\
        htmlexport_global.h

OTHER_FILES += \
    htmlexport.json \
    htmlexport.css

RESOURCES += \
    htmlexport.qrc

FORMS += \
    htmlexport.ui


TRANSLATIONS += translations/HtmlExport_ro_RO.ts \
		translations/HtmlExport_de_DE.ts \
		translations/HtmlExport_it_IT.ts \
		translations/HtmlExport_zh_CN.ts \
		translations/HtmlExport_sk_SK.ts \
		translations/HtmlExport_ru_RU.ts \
		translations/HtmlExport_pt_BR.ts \
		translations/HtmlExport_fr_FR.ts \
		translations/HtmlExport_es_ES.ts \
		translations/HtmlExport_pl_PL.ts


























