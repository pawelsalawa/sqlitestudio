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

isEmpty(PYTHON_VERSION) {
    PYTHON_VERSION = 3.9
}

linux: {
    LIBS += -lpython$$PYTHON_VERSION
}

macx: {
    LIBS += -lpython$$PYTHON_VERSION
}

win32: {
    INCLUDEPATH += $$PWD/../../../include/python
    LIBS += -lpython$$replace(PYTHON_VERSION, \., ) -L$$PWD/../../../lib/python
}

RESOURCES += \
    scriptingpython.qrc
