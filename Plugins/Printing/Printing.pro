#-------------------------------------------------
#
# Project created by QtCreator 2014-08-03T20:25:45
#
#-------------------------------------------------

QT += printsupport

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = Printing
TEMPLATE = lib

DEFINES += PRINTING_LIBRARY

SOURCES += printing.cpp \
    printingexport.cpp

HEADERS += printing.h\
        printing_global.h \
    printingexport.h

OTHER_FILES += \
    printing.json

INCLUDEPATH += $$PLUGINSDIR/PdfExport
DEPENDPATH += $$PLUGINSDIR/PdfExport

win32|macx: {
#    LIBS += -lPdfExport
    pluginDep(PdfExport)
}

RESOURCES += \
    printing.qrc


TRANSLATIONS += translations/Printing.ts \
		translations/Printing_ro_RO.ts \
		translations/Printing_de_DE.ts \
		translations/Printing_it_IT.ts \
		translations/Printing_zh_CN.ts \
		translations/Printing_sk_SK.ts \
		translations/Printing_ru_RU.ts \
		translations/Printing_pt_BR.ts \
		translations/Printing_fr_FR.ts \
		translations/Printing_es_ES.ts \
		translations/Printing_pl_PL.ts



























