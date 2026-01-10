QT += gui widgets

include($$PWD/../../SQLiteStudio3/plugins.pri)

TEMPLATE = lib
TARGET = ErdEditor

DEFINES += ERDEDITOR_LIBRARY

SOURCES += \
    changes/erdchangemoveentity.cpp \
    scene/erdarrowitem.cpp \
    changes/erdchange.cpp \
    changes/erdchangecomposite.cpp \
    changes/erdchangedeleteconnection.cpp \
    changes/erdchangedeleteentity.cpp \
    changes/erdchangeentity.cpp \
    changes/erdchangenewentity.cpp \
    changes/erdchangeregistry.cpp \
    changes/erdchangeregistrydialog.cpp \
    panel/erdcolumnfkpanel.cpp \
    scene/erdconnection.cpp \
    panel/erdconnectionpanel.cpp \
    scene/erdcurvyarrowitem.cpp \
    erdeditorplugin.cpp \
    scene/erdentity.cpp \
    erdgraphvizlayoutplanner.cpp \
    scene/erditem.cpp \
    scene/erdlinearrowitem.cpp \
    scene/erdscene.cpp \
    scene/erdsquarearrowitem.cpp \
    panel/erdtablefkpanel.cpp \
    panel/erdtablewindow.cpp \
    scene/erdview.cpp \
    erdwindow.cpp

HEADERS += \
    changes/erdchangemoveentity.h \
    scene/erdarrowitem.h \
    changes/erdchange.h \
    changes/erdchangecomposite.h \
    changes/erdchangedeleteconnection.h \
    changes/erdchangedeleteentity.h \
    changes/erdchangeentity.h \
    changes/erdchangenewentity.h \
    changes/erdchangeregistry.h \
    changes/erdchangeregistrydialog.h \
    panel/erdcolumnfkpanel.h \
    scene/erdconnection.h \
    panel/erdconnectionpanel.h \
    scene/erdcurvyarrowitem.h \
    erdeditor_global.h \
    erdeditorplugin.h \
    scene/erdentity.h \
    erdgraphvizlayoutplanner.h \
    scene/erditem.h \
    scene/erdlinearrowitem.h \
    panel/erdpropertiespanel.h \
    scene/erdscene.h \
    scene/erdsquarearrowitem.h \
    panel/erdtablefkpanel.h \
    panel/erdtablewindow.h \
    scene/erdview.h \
    erdwindow.h

OTHER_FILES += \
    ErdEditor.json

LIBS += -lgvc -lcdt -lcgraph

FORMS += \
    changes/erdchangeregistrydialog.ui \
    erdconfig.ui \
    panel/erdconnectionpanel.ui \
    erdwindow.ui

RESOURCES += \
    erdeditor.qrc
