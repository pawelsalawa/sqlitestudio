CONFIG += c++11

DESTDIR = $$PWD/../output/SQLiteStudio/plugins
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

INCLUDEPATH += $$PWD/coreSQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

PLUGINSDIR = $$PWD/../Plugins
INCLUDEPATH += $$PLUGINSDIR
DEPENDPATH += $$PLUGINSDIR

export (PLUGINSDIR)

contains(QT, gui) {
    INCLUDEPATH += $$PWD/guiSQLiteStudio
    INCLUDEPATH += $$UI_DIR/guiSQLiteStudio
    DEPENDPATH += $$PWD/guiSQLiteStudio
}

win32: {
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib -L$$DESTDIR/.. -lcoreSQLiteStudio -L$$PWD/../output/SQLiteStudio/plugins

    contains(QT, gui) {
        LIBS += -lguiSQLiteStudio
    }
}

unix: {
    target.path = /usr/lib/sqlitestudio
    INSTALLS += target
}

macx: {
    GUI_APP = $$PWD/../output/SQLiteStudio/SQLiteStudio.app/Contents/MacOS/SQLiteStudio
    export (GUI_APP)

    LIBS += -L$$PWD/../output/SQLiteStudio -lcoreSQLiteStudio
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib -L$$DESTDIR
    QMAKE_CXXFLAGS += -stdlib=libc++ -mmacosx-version-min=10.7
}

win32|macx: {
    CONFIG += portable

    contains(QT, gui) {
        LIBS += -lguiSQLiteStudio
    }
}

portable {
    QMAKE_LFLAGS += -Wl,-rpath,.. -Wl,-rpath,$$DESTDIR/..
}
