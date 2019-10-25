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


TRANSLATIONS += translations/SqlFormatterSimple_ro_RO.ts \
		translations/SqlFormatterSimple_de_DE.ts \
		translations/SqlFormatterSimple_it_IT.ts \
		translations/SqlFormatterSimple_zh_CN.ts \
		translations/SqlFormatterSimple_sk_SK.ts \
		translations/SqlFormatterSimple_ru_RU.ts \
		translations/SqlFormatterSimple_pt_BR.ts \
		translations/SqlFormatterSimple_fr_FR.ts \
		translations/SqlFormatterSimple_es_ES.ts \
		translations/SqlFormatterSimple_pl_PL.ts


























