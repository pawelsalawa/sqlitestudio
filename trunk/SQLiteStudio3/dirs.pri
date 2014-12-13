release {
    OUTPUT_DIR_NAME = output
}
debug {
    OUTPUT_DIR_NAME = output_debug
}

DESTDIR = $$PWD/../$$OUTPUT_DIR_NAME/SQLiteStudio
OBJECTS_DIR = $$PWD/../$$OUTPUT_DIR_NAME/build
MOC_DIR = $$PWD/../$$OUTPUT_DIR_NAME/build
UI_DIR = $$PWD/../$$OUTPUT_DIR_NAME/build

LIBS += -L$$DESTDIR


macx: {
    QMAKE_CXXFLAGS += -Wno-gnu-zero-variadic-macro-arguments -Wno-overloaded-virtual
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib
}

win32: {
    INCLUDEPATH += $$PWD/../../include $$PWD/../../include/quazip
    LIBS += -L$$PWD/../../lib
}

INCLUDEPATH += $$PWD/coreSQLiteStudio
DEPENDPATH += $$PWD/coreSQLiteStudio

contains(QT, gui): {
    INCLUDEPATH += $$PWD/guiSQLiteStudio $$PWD/../$$OUTPUT_DIR_NAME/build/guiSQLiteStudio
    DEPENDPATH += $$PWD/guiSQLiteStudio
}

win32|macx: {
    CONFIG += portable
}

portable {
    QMAKE_LFLAGS += -Wl,-rpath,.
    linux: {
        LIBS += -L$$DESTDIR/lib
    }
}

unix: {
    isEmpty(LIBDIR) {
	LIBDIR = /usr/lib
    }
    export(LIBDIR)
    isEmpty(BINDIR) {
	BINDIR = /usr/bin
    }
    export(BINDIR)
}
