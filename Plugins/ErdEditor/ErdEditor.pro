QT += gui widgets

include($$PWD/../../SQLiteStudio3/plugins.pri)

TEMPLATE = lib
TARGET = ErdEditor

DEFINES += ERDEDITOR_LIBRARY

SOURCES += \
    erdarrowitem.cpp \
    erdchange.cpp \
    erdchangeentity.cpp \
    erdchangelayout.cpp \
    erdchangeregistry.cpp \
    erdconnection.cpp \
    erdcurvyarrowitem.cpp \
    erdeditorplugin.cpp \
    erdentity.cpp \
    erdgraphvizlayoutplanner.cpp \
    erditem.cpp \
    erdlinearrowitem.cpp \
    erdscene.cpp \
    erdsquarearrowitem.cpp \
    erdtablewindow.cpp \
    erdview.cpp \
    erdwindow.cpp

HEADERS += \
    erdarrowitem.h \
    erdchange.h \
    erdchangeentity.h \
    erdchangelayout.h \
    erdchangeregistry.h \
    erdconnection.h \
    erdcurvyarrowitem.h \
    erdeditor_global.h \
    erdeditorplugin.h \
    erdentity.h \
    erdgraphvizlayoutplanner.h \
    erditem.h \
    erdlinearrowitem.h \
    erdscene.h \
    erdsquarearrowitem.h \
    erdtablewindow.h \
    erdview.h \
    erdwindow.h

OTHER_FILES += \
    ErdEditor.json

LIBS += -lgvc -lcdt -lcgraph

FORMS += \
    erdconfig.ui \
    erdwindow.ui

RESOURCES += \
    erdeditor.qrc
