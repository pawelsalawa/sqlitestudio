QT += gui widgets

include($$PWD/../../SQLiteStudio3/plugins.pri)

TEMPLATE = lib
TARGET = ErdEditor

DEFINES += ERDEDITOR_LIBRARY

SOURCES += \
    erdeditorplugin.cpp \
    erdeditorwindow.cpp \
    erdscene.cpp

HEADERS += \
    erdeditor_global.h \
    erdeditorplugin.h \
    erdeditorwindow.h \
    erdscene.h

OTHER_FILES += \
    ErdEditor.json

win32: {
    LIBS += -lcoreSQLiteStudio -lguiSQLiteStudio
}

FORMS += \
    erdeditorwindow.ui

RESOURCES += \
    erdeditor.qrc
