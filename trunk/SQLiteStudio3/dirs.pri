DESTDIR = $$PWD/../output/SQLiteStudio
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

LIBS += -L$$DESTDIR
INCLUDEPATH += $$PWD/coreSQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

CONFIG(portable): {
    QMAKE_LFLAGS += -Wl,-rpath,. -Wl,-rpath,$$DESTDIR
}
