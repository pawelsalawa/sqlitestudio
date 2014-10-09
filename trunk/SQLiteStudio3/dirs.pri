DESTDIR = $$PWD/../output/SQLiteStudio
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

LIBS += -L$$DESTDIR -L$$DESTDIR/lib

macx: {
    QMAKE_CXXFLAGS += -Wno-gnu-zero-variadic-macro-arguments -Wno-overloaded-virtual
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib
}

win32: {
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib
}

INCLUDEPATH += $$PWD/coreSQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

contains(QT, gui): {
    INCLUDEPATH += $$PWD/guiSQLiteStudio $$PWD/../output/build/guiSQLiteStudio
    DEPENDPATH += $$PWD/guiSQLiteStudio
}

win32|macx: {
    CONFIG += portable
}

portable {
    QMAKE_LFLAGS += -Wl,-rpath,.
    # -Wl,-rpath,$$DESTDIR
}
