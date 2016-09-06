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


TRANSLATIONS += PdfExport_de.ts \
		PdfExport_it.ts \
		PdfExport_zh_CN.ts \
		PdfExport_sk.ts \
		PdfExport_ru.ts \
		PdfExport_pt_BR.ts \
		PdfExport_fr.ts \
		PdfExport_es.ts \
		PdfExport_pl.ts















