QT += gui widgets

include($$PWD/../../SQLiteStudio3/plugins.pri)

TEMPLATE = lib
TARGET = ErdEditor

DEFINES += ERDEDITOR_LIBRARY

SOURCES += \
    erdarrowitem.cpp \
    erdconnection.cpp \
    erdeditorplugin.cpp \
    erdeditorwindow.cpp \
    erdentity.cpp \
    erditem.cpp \
    erdscene.cpp \
    erdview.cpp

HEADERS += \
    erdarrowitem.h \
    erdconnection.h \
    erdeditor_global.h \
    erdeditorplugin.h \
    erdeditorwindow.h \
    erdentity.h \
    erditem.h \
    erdscene.h \
    erdview.h

OTHER_FILES += \
    ErdEditor.json

win32: {
    LIBS += -lcoreSQLiteStudio -lguiSQLiteStudio
}

FORMS += \
    erdeditorwindow.ui

RESOURCES += \
    erdeditor.qrc
