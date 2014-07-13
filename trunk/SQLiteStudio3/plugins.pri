QMAKE_CXXFLAGS += -std=c++11

DESTDIR = $$PWD/../output/SQLiteStudio/plugins
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

INCLUDEPATH += $$PWD/coreSQLiteStudio
INCLUDEPATH += $$PWD/SQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

win32: {
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib -L$$DESTDIR/.. -lcoreSQLiteStudio -L$$PWD/../output/SQLiteStudio/plugins
}

unix: {
    target.path = /usr/lib/sqlitestudio
    INSTALLS += target
}

defineTest(guiPlugin) {
    win32: {
        LIBS += -Wl,$$DESTDIR/../libSQLiteStudio.a
    }
}
