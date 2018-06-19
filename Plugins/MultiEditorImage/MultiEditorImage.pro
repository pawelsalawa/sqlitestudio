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

TRANSLATIONS += \
		MultiEditorImage_ro_RO.ts\
		MultiEditorImage_de.ts\
		MultiEditorImage_it.ts\
		MultiEditorImage_zh_CN.ts\
		MultiEditorImage_sk.ts\
		MultiEditorImage_ru.ts\
		MultiEditorImage_pt_BR.ts\
		MultiEditorImage_fr.ts\
		MultiEditorImage_es.ts\
		MultiEditorImage_pl.ts

