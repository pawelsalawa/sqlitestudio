#-------------------------------------------------
#
# Project created by QtCreator 2014-09-06T00:39:26
#
#-------------------------------------------------

QT       += core gui widgets network

include($$PWD/../common.pri)
include($$PWD/../utils.pri)
include($$PWD/../version.pri)

OBJECTS_DIR = $$OBJECTS_DIR/sqlitestudio
MOC_DIR = $$MOC_DIR/sqlitestudio
UI_DIR = $$UI_DIR/sqlitestudio

linux {
    TARGET = sqlitestudio
} else {
    TARGET = SQLiteStudio
}
TEMPLATE = app

CONFIG  += c++20
QMAKE_CXXFLAGS += -pedantic

DEFINES += QAPPLICATION_CLASS=QApplication

win32 {
    msvc:LIBS += Advapi32.lib
    gcc:LIBS += -lAdvapi32
}

portable {
    DEFINES += PORTABLE_CONFIG
    linux {
        QMAKE_LFLAGS += -Wl,-rpath,./lib
    }
}

LIBS += -lcoreSQLiteStudio -lguiSQLiteStudio

SOURCES += main.cpp \
    singleapplication/singleapplication.cpp

win32 {
    DQ = $$escape_expand(\")
    QMAKE_SUBSTITUTES += windows.rc.in
    RC_FILE = $$OUT_PWD/windows.rc
    msvc:LIBS += User32.lib
    gcc:LIBS += -lUser32
}

macx {
    ICON = ../guiSQLiteStudio/img/sqlitestudio.icns
}

OTHER_FILES += \
    SQLiteStudio.exe.manifest \
    windows.rc.in

unix {
    target.path = $$BINDIR
    INSTALLS += target

    icon_scalable.path = $$DATADIR/icons/hicolor/scalable/apps
    icon_scalable.files = ../guiSQLiteStudio/img/sqlitestudio.svg
    INSTALLS += icon_scalable

    OTHER_FILES += ../guiSQLiteStudio/img/sqlitestudio_16.png
    icon_16.path = $$DATADIR/icons/hicolor/16x16/apps
    icon_16.files = $$OBJECTS_DIR/icons/16x16/sqlitestudio.png
    icon_16.extra = install -D -m 644 ../guiSQLiteStudio/img/sqlitestudio_16.png "$$OBJECTS_DIR/icons/16x16/sqlitestudio.png"
    INSTALLS += icon_16

    OTHER_FILES += ../guiSQLiteStudio/img/sqlitestudio_48.png
    icon_48.path = $$DATADIR/icons/hicolor/48x48/apps
    icon_48.files = $$OBJECTS_DIR/icons/48x48/sqlitestudio.png
    icon_48.extra = install -D -m 644 ../guiSQLiteStudio/img/sqlitestudio_48.png "$$OBJECTS_DIR/icons/48x48/sqlitestudio.png"
    INSTALLS += icon_48

    OTHER_FILES += ../guiSQLiteStudio/img/sqlitestudio_256.png
    icon_256.path = $$DATADIR/icons/hicolor/256x256/apps
    icon_256.files = $$OBJECTS_DIR/icons/256x256/sqlitestudio.png
    icon_256.extra = install -D -m 644 ../guiSQLiteStudio/img/sqlitestudio_256.png "$$OBJECTS_DIR/icons/256x256/sqlitestudio.png"
    INSTALLS += icon_256
}

RESOURCES += \
    sqlitestudio.qrc

HEADERS += \
    singleapplication/singleapplication.h \
    singleapplication/singleapplication_p.h


























