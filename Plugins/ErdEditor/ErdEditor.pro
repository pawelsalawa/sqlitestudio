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
    erdchangenewentity.cpp \
    erdchangeregistry.cpp \
    erdcolumnfkpanel.cpp \
    erdconnection.cpp \
    erdconnectionpanel.cpp \
    erdcurvyarrowitem.cpp \
    erdeditorplugin.cpp \
    erdentity.cpp \
    erdgraphvizlayoutplanner.cpp \
    erditem.cpp \
    erdlinearrowitem.cpp \
    erdscene.cpp \
    erdsquarearrowitem.cpp \
    erdtablefkpanel.cpp \
    erdtablewindow.cpp \
    erdview.cpp \
    erdwindow.cpp

HEADERS += \
    erdarrowitem.h \
    erdchange.h \
    erdchangeentity.h \
    erdchangelayout.h \
    erdchangenewentity.h \
    erdchangeregistry.h \
    erdcolumnfkpanel.h \
    erdconnection.h \
    erdconnectionpanel.h \
    erdcurvyarrowitem.h \
    erdeditor_global.h \
    erdeditorplugin.h \
    erdentity.h \
    erdgraphvizlayoutplanner.h \
    erditem.h \
    erdlinearrowitem.h \
    erdscene.h \
    erdsquarearrowitem.h \
    erdtablefkpanel.h \
    erdtablewindow.h \
    erdview.h \
    erdwindow.h

OTHER_FILES += \
    ErdEditor.json

LIBS += -lgvc -lcdt -lcgraph

FORMS += \
    erdconfig.ui \
    erdconnectionpanel.ui \
    erdwindow.ui

RESOURCES += \
    erdeditor.qrc
