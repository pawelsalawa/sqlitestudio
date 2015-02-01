#-------------------------------------------------
#
# Project created by QtCreator 2014-09-06T00:39:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/sqlitestudio
MOC_DIR = $$MOC_DIR/sqlitestudio
UI_DIR = $$UI_DIR/sqlitestudio

linux: {
    TARGET = sqlitestudio
}
!linux: {
    TARGET = SQLiteStudio
}
TEMPLATE = app

CONFIG   += c++11
QMAKE_CXXFLAGS += -pedantic
linux|portable {
    QMAKE_LFLAGS += -Wl,-rpath,./lib
}

LIBS += -lcoreSQLiteStudio -lguiSQLiteStudio

SOURCES += main.cpp

TRANSLATIONS += translations/sqlitestudio_ru.ts \
		translations/sqlitestudio_pt_BR.ts \
		translations/sqlitestudio_fr.ts \
		translations/sqlitestudio_es.ts \
		translations/sqlitestudio_pl.ts

win32: {
    RC_FILE = windows.rc
}

macx: {
    ICON = ../guiSQLiteStudio/img/sqlitestudio.icns
}

OTHER_FILES += \
    windows.rc \
    SQLiteStudio.exe.manifest

unix: {
    target.path = $$BINDIR
    INSTALLS += target
}

RESOURCES += \
    sqlitestudio.qrc








