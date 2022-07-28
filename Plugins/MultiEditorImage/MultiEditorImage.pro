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
