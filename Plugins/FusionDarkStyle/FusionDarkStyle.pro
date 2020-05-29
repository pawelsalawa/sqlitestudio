include($$PWD/../../SQLiteStudio3/plugins.pri)

DESTDIR = $$PWD/../../$$OUTPUT_DIR_NAME/SQLiteStudio/styles
mkpath($$DESTDIR)

QT += widgets

TEMPLATE = lib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    fusiondarkplugin.cpp

HEADERS += \
    fusiondarkplugin.h

DISTFILES += FusionDarkStyle.json
