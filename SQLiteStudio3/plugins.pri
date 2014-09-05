CONFIG += c++11

macx: {
    QMAKE_CXXFLAGS += -stdlib=libc++ -mmacosx-version-min=10.7
}

DESTDIR = $$PWD/../output/SQLiteStudio/plugins
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build
INCLUDEPATH += $$UI_DIR/SQLiteStudio

INCLUDEPATH += $$PWD/coreSQLiteStudio
INCLUDEPATH += $$PWD/SQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

PLUGINSDIR = $$PWD/../Plugins
INCLUDEPATH += $$PLUGINSDIR
DEPENDPATH += $$PLUGINSDIR

export (PLUGINSDIR)

win32: {
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib -L$$DESTDIR/.. -lcoreSQLiteStudio -L$$PWD/../output/SQLiteStudio/plugins
    LIBS += -Wl,$$DESTDIR/../libSQLiteStudio.a
}

unix: {
    target.path = /usr/lib/sqlitestudio
    INSTALLS += target
}

macx: {
    GUI_APP = $$PWD/../output/SQLiteStudio/SQLiteStudio.app/Contents/MacOS/SQLiteStudio
    export (GUI_APP)

    LIBS += -L$$PWD/../output/SQLiteStudio/SQLiteStudio.app/Contents/MacOS -lcoreSQLiteStudio
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib -L$$DESTDIR

    CONFIG += plugin

    contains(QT, gui) {
        QMAKE_LFLAGS_SHLIB -= -dynamiclib
        QMAKE_LFLAGS_PLUGIN -= -dynamiclib
        QMAKE_LFLAGS += -bundle -bundle_loader $$GUI_APP
    }
}

win32|macx: {
    CONFIG += portable
}

portable {
    QMAKE_LFLAGS += -Wl,-rpath,.. -Wl,-rpath,$$DESTDIR/..
}
