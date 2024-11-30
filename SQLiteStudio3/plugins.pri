include($$PWD/common.pri)

CONFIG += c++20 plugin

DESTDIR = $$PWD/../$$OUTPUT_DIR_NAME/SQLiteStudio/plugins
OBJECTS_DIR = $$PWD/../$$OUTPUT_DIR_NAME/build
MOC_DIR = $$PWD/../$$OUTPUT_DIR_NAME/build
UI_DIR = $$PWD/../$$OUTPUT_DIR_NAME/build

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
    LIBS += -L$$PWD/../../lib -L$$DESTDIR/.. -lcoreSQLiteStudio -L$$PWD/../$$OUTPUT_DIR_NAME/SQLiteStudio/plugins

    defineTest(pluginDep) {
        linker_flag_parts = -l $$1
        linker_flag = $$join(linker_flag_parts)
        LIBS += $$linker_flag
        export(LIBS)
    }

    defineTest(frameworkDep) {
        linker_flag_parts = -l $$1
        linker_flag = $$join(linker_flag_parts)
        LIBS += $$linker_flag
        export(LIBS)
    }

    # Duplicated later on, not sure why yet.
    #contains(QT, gui) {
    #    LIBS += -lguiSQLiteStudio
    #}

    SO_NAME = ddl
    SO_PREFIX = lib
    PLATFORM = win32
    export(SO_NAME)
    export(SO_PREFIX)
    export(PLATFORM)
}

unix: {
    defineTest(pluginDep) {
    }

    defineTest(frameworkDep) {
        linker_flag_parts = -l $$1
        linker_flag = $$join(linker_flag_parts)
        LIBS += $$linker_flag
        export(LIBS)
    }

    target.path = $$LIBDIR/sqlitestudio
    INSTALLS += target
}

linux: {
    SO_NAME = so
    SO_PREFIX = lib
    PLATFORM = linux32
    equals(QMAKE_HOST.arch, "x86_64") {
        PLATFORM = linux64
    }
    export(SO_NAME)
    export(SO_PREFIX)
    export(PLATFORM)
}

macx: {
    PLUGINSDIR = $$PWD/../PlugIns
    GUI_APP = $$PWD/../$$OUTPUT_DIR_NAME/SQLiteStudio/SQLiteStudio.app/Contents/MacOS/SQLiteStudio
    export (GUI_APP)

    LIBS += -L$$PWD/../$$OUTPUT_DIR_NAME/SQLiteStudio -lcoreSQLiteStudio
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib -L$$DESTDIR
    QMAKE_CXXFLAGS += -stdlib=libc++ -mmacosx-version-min=10.7

    defineTest(pluginDep) {
        out_file_parts = $$DESTDIR/lib $$TARGET .dylib
        out_file = $$join(out_file_parts)
        lib_name_parts = lib $$1 .dylib
        lib_name = $$join(lib_name_parts)
        QMAKE_POST_LINK += "install_name_tool -change $$lib_name @loader_path/../PlugIns/$$lib_name \"$$out_file\";"
        export(QMAKE_POST_LINK)

        linker_flag_parts = -l $$1
        linker_flag = $$join(linker_flag_parts)
        LIBS += $$linker_flag
        export(LIBS)
    }

    defineTest(frameworkDep) {
        out_file_parts = $$DESTDIR/lib $$TARGET .dylib
        out_file = $$join(out_file_parts)
        lib_name_parts = lib $$1 .dylib
        lib_name = $$join(lib_name_parts)
        QMAKE_POST_LINK += "install_name_tool -change $$lib_name @loader_path/../Frameworks/$$lib_name \"$$out_file\";"
        export(QMAKE_POST_LINK)

        linker_flag_parts = -l $$1
        linker_flag = $$join(linker_flag_parts)
        LIBS += $$linker_flag
        export(LIBS)
    }

    SO_NAME = dylib
    SO_PREFIX = lib
    PLATFORM = macosx
    export(SO_NAME)
    export(SO_PREFIX)
    export(PLATFROM)
}

win32|macx: {
    CONFIG += portable

    contains(QT, gui) {
        LIBS += -lguiSQLiteStudio
    }
}

portable {
    QMAKE_LFLAGS += -Wl,-rpath,. -Wl,-rpath,..
    linux: {
        QMAKE_LFLAGS += -Wl,-rpath,../lib
    }
}
