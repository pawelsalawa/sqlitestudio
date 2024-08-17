QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = ScriptingPython
TEMPLATE = lib

DEFINES += SCRIPTINGPYTHON_LIBRARY

SOURCES += scriptingpython.cpp

HEADERS += scriptingpython.h\
        scriptingpython_global.h

OTHER_FILES += \
    scriptingpython.json

portable {
    CONFIG += dynamic_python
}

dynamic_python {
    DEFINES += PYTHON_DYNAMIC_BINDING
    SOURCES += dynamicpythonapi.cpp
    HEADERS += dynamicpythonapi.h
}

!dynamic_python {
    HEADERS += staticpythonapi.h

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
        LIBS += -lpython$$replace(PYTHON_VERSION, \., ) -L$$PWD/../../../lib/python
    }
}

win32: {
    INCLUDEPATH += $$PWD/../../../include/python
}

FORMS += \
    scriptingpython.ui

RESOURCES += \
    scriptingpython.qrc
