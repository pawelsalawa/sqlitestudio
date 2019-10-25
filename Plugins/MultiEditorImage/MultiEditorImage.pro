include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TARGET = MultiEditorImage
TEMPLATE = lib

DEFINES += MULTIEDITORIMAGE_LIBRARY

SOURCES += multieditorimage.cpp

HEADERS += multieditorimage.h\
        multieditorimage_global.h

OTHER_FILES += \
    multieditorimage.json

RESOURCES += \
    multieditorimage.qrc

TRANSLATIONS += translations/MultiEditorImage.ts \
		\
		translations/MultiEditorImage_ro_RO.ts\
		translations/MultiEditorImage_de_DE.ts\
		translations/MultiEditorImage_it_IT.ts\
		translations/MultiEditorImage_zh_CN.ts\
		translations/MultiEditorImage_sk_SK.ts\
		translations/MultiEditorImage_ru_RU.ts\
		translations/MultiEditorImage_pt_BR.ts\
		translations/MultiEditorImage_fr_FR.ts\
		translations/MultiEditorImage_es_ES.ts\
		translations/MultiEditorImage_pl_PL.ts












