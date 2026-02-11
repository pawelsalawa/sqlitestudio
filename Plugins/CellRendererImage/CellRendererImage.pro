include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TEMPLATE = lib
DEFINES += CELLRENDERERIMAGE_LIBRARY

SOURCES += \
    cellrendererimage.cpp \
    cellrendererimageplugin.cpp

HEADERS += \
    cellrendererimage.h \
    cellrendererimage_global.h \
    cellrendererimageplugin.h

OTHER_FILES += \
    cellrendererimage.json

FORMS += \
    cellrendererimage.ui

RESOURCES += \
    cellrendererimage.qrc

