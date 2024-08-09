#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T05:20:57
#
#-------------------------------------------------

QT       -= gui

versionAtLeast(QT_VERSION, 6.0.0) {
    QT += core5compat
}

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = XmlExport
TEMPLATE = lib

DEFINES += XMLEXPORT_LIBRARY

SOURCES += xmlexport.cpp

HEADERS += xmlexport.h\
        xmlexport_global.h

FORMS += XmlExport.ui

OTHER_FILES += \
    xmlexport.json

RESOURCES += \
    xmlexport.qrc
