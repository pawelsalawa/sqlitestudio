DESTDIR = $$PWD/../output/SQLiteStudio
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

LIBS += -L$$DESTDIR

macx: {
    QMAKE_CXXFLAGS += -Wno-gnu-zero-variadic-macro-arguments
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib
}

INCLUDEPATH += $$PWD/coreSQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

win32|macx: {
    CONFIG += portable
}

portable {
    QMAKE_LFLAGS += -Wl,-rpath,. -Wl,-rpath,$$DESTDIR
}
