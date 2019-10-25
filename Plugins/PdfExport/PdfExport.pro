#-------------------------------------------------
#
# Project created by QtCreator 2014-06-23T01:06:41
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = PdfExport
TEMPLATE = lib

DEFINES += PDFEXPORT_LIBRARY

SOURCES += pdfexport.cpp

HEADERS += pdfexport.h\
        pdfexport_global.h

OTHER_FILES += \
    pdfexport.json

FORMS += \
    pdfexport.ui

RESOURCES += \
    pdfexport.qrc


TRANSLATIONS += translations/PdfExport.ts \
		translations/PdfExport_ro_RO.ts \
		translations/PdfExport_de_DE.ts \
		translations/PdfExport_it_IT.ts \
		translations/PdfExport_zh_CN.ts \
		translations/PdfExport_sk_SK.ts \
		translations/PdfExport_ru_RU.ts \
		translations/PdfExport_pt_BR.ts \
		translations/PdfExport_fr_FR.ts \
		translations/PdfExport_es_ES.ts \
		translations/PdfExport_pl_PL.ts



























