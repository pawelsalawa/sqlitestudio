DESTDIR = $$PWD/../output/SQLiteStudio/plugins
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

INCLUDEPATH += $$PWD/coreSQLiteStudio
INCLUDEPATH += $$PWD/SQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

win32: {
    INCLUDEPATH += $$PWD/../include
    LIBS += -L$$PWD/../lib -L$$DESTDIR/.. -lcoreSQLiteStudio
}

unix: {
    target.path = /usr/lib/sqlitestudio
    INSTALLS += target
}
