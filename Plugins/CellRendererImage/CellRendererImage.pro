include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TEMPLATE = lib
DEFINES += CELLRENDERERIMAGE_LIBRARY

SOURCES += \
    cellrendererimage.cpp

HEADERS += \
    cellrendererimage.h \
    cellrendererimage_global.h

OTHER_FILES += \
    cellrendererimage.json

