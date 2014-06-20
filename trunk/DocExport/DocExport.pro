#-------------------------------------------------
#
# Project created by QtCreator 2014-06-20T22:19:47
#
#-------------------------------------------------

include($$PWD/../SQLiteStudio3/plugins.pri)
include($$PWD/../SQLiteStudio3/utils.pri)

QMAKE_CXXFLAGS += -std=c++11

TARGET = DocExport
TEMPLATE = lib

DEFINES += DOCEXPORT_LIBRARY

SOURCES += docexport.cpp

HEADERS += docexport.h\
        docexport_global.h

OTHER_FILES += \
    docexport.json
