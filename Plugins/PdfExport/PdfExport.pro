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
