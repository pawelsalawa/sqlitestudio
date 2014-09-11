CONFIG += c++11 plugin

DESTDIR = $$PWD/../output/SQLiteStudio/plugins
OBJECTS_DIR = $$PWD/../output/build
MOC_DIR = $$PWD/../output/build
UI_DIR = $$PWD/../output/build

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
    LIBS += -L$$PWD/../../lib -L$$DESTDIR/.. -lcoreSQLiteStudio -L$$PWD/../output/SQLiteStudio/plugins

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

    contains(QT, gui) {
        LIBS += -lguiSQLiteStudio
    }
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

    target.path = /usr/lib/sqlitestudio
    INSTALLS += target
}

macx: {
    GUI_APP = $$PWD/../output/SQLiteStudio/SQLiteStudio.app/Contents/MacOS/SQLiteStudio
    export (GUI_APP)

    LIBS += -L$$PWD/../output/SQLiteStudio -lcoreSQLiteStudio
    INCLUDEPATH += $$PWD/../../include
    LIBS += -L$$PWD/../../lib -L$$DESTDIR
    QMAKE_CXXFLAGS += -stdlib=libc++ -mmacosx-version-min=10.7

    defineTest(pluginDep) {
        out_file_parts = $$DESTDIR/lib $$TARGET .dylib
        out_file = $$join(out_file_parts)
        lib_name_parts = lib $$1 .dylib
        lib_name = $$join(lib_name_parts)
        QMAKE_POST_LINK += install_name_tool -change $$lib_name @loader_path/../plugins/$$lib_name $$out_file
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
        QMAKE_POST_LINK += install_name_tool -change $$lib_name @loader_path/../Frameworks/$$lib_name $$out_file
        export(QMAKE_POST_LINK)

        linker_flag_parts = -l $$1
        linker_flag = $$join(linker_flag_parts)
        LIBS += $$linker_flag
        export(LIBS)
    }
}

win32|macx: {
    CONFIG += portable

    contains(QT, gui) {
        LIBS += -lguiSQLiteStudio
    }
}

portable {
    QMAKE_LFLAGS += -Wl,-rpath,. -Wl,-rpath,..
}
