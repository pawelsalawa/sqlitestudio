#-------------------------------------------------
#
# Project created by QtCreator 2013-12-02T16:14:12
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = SqlFormatterSimple
TEMPLATE = lib

DEFINES += SQLFORMATTERSIMPLE_LIBRARY

SOURCES += sqlformattersimpleplugin.cpp

HEADERS += sqlformattersimpleplugin.h\
        sqlformattersimple_global.h

FORMS += \
    SqlFormatterSimple.ui

OTHER_FILES += \
    sqlformattersimple.json

RESOURCES += \
    sqlformattersimple.qrc


TRANSLATIONS += SqlFormatterSimple_sk.ts \
		SqlFormatterSimple_de.ts \
		SqlFormatterSimple_ru.ts \
		SqlFormatterSimple_pt_BR.ts \
		SqlFormatterSimple_fr.ts \
		SqlFormatterSimple_es.ts \
		SqlFormatterSimple_pl.ts











