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


TRANSLATIONS += HtmlExport_de.ts \
		HtmlExport_it.ts \
		HtmlExport_zh_CN.ts \
		HtmlExport_sk.ts \
		HtmlExport_ru.ts \
		HtmlExport_pt_BR.ts \
		HtmlExport_fr.ts \
		HtmlExport_es.ts \
		HtmlExport_pl.ts















