OUTPUT_DIR_NAME = output
export(OUTPUT_DIR_NAME)

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
    INCLUDEPATH += $$PWD/../../include
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
	LIBDIR = $$PREFIX/lib
    }
    export(LIBDIR)
    isEmpty(BINDIR) {
	BINDIR = $$PREFIX/bin
    }
    export(BINDIR)
}

# Enable automatic translation files processing globally
QMAKE_RESOURCE_FLAGS += -name $${TARGET}_${QMAKE_FILE_BASE}
TRANSLATIONS += $$files($$_PRO_FILE_PWD_/translations/*.ts)
defined(TARGET, "var") {
    DEFINES += "PROJECT_MODULE_NAME=$${TARGET}"
}
!isEmpty(TRANSLATIONS) {
    CONFIG += lrelease embed_translations
    QM_FILES_RESOURCE_PREFIX = /msg/translations
}

