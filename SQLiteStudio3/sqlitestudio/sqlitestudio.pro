#-------------------------------------------------
#
# Project created by QtCreator 2014-09-06T00:39:26
#
#-------------------------------------------------

QT       += core gui widgets network

include($$PWD/../common.pri)
include($$PWD/../utils.pri)

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
    RC_FILE = windows.rc
    msvc:LIBS += User32.lib
    gcc:LIBS += -lUser32
}

macx {
    ICON = ../guiSQLiteStudio/img/sqlitestudio.icns
}

OTHER_FILES += \
    windows.rc \
    SQLiteStudio.exe.manifest

unix {
    target.path = $$BINDIR
    INSTALLS += target
}

RESOURCES += \
    sqlitestudio.qrc

HEADERS += \
    singleapplication/singleapplication.h \
    singleapplication/singleapplication_p.h


























