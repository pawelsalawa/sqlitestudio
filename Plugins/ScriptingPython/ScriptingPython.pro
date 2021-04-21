QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = ScriptingPython
TEMPLATE = lib

DEFINES += SCRIPTINGPYTHON_LIBRARY

QMAKE_CXXFLAGS += -Wno-cast-function-type

SOURCES += scriptingpython.cpp

HEADERS += scriptingpython.h\
        scriptingpython_global.h

OTHER_FILES += \
    scriptingpython.json

linux: {
}

macx: {
}

win32: {
    INCLUDEPATH += $$PWD/../../../include/python
    LIBS += -lpython39 -L$$PWD/../../../lib/python
}

RESOURCES += \
    scriptingpython.qrc

TRANSLATIONS += $$files(translations/*.ts)



























