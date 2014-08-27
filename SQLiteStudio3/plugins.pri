QMAKE_CXXFLAGS += -std=c++11

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

portable {
    QMAKE_LFLAGS += -Wl,-rpath,.. -Wl,-rpath,$$DESTDIR/..
}
