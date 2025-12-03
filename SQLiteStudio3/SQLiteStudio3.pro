TEMPLATE = subdirs

REQ_QT_MAJOR = 5
REQ_QT_MINOR = 12
REQ_QT_PATCH = 0

!versionAtLeast(QT_VERSION, $${REQ_QT_MAJOR}.$${REQ_QT_MINOR}.$${REQ_QT_PATCH}) {
    error($$sprintf("Required Qt version is at least %1.%2.%3. This Qt version is %4.%5.%6.", \
        $$REQ_QT_MAJOR, $$REQ_QT_MINOR, $$REQ_QT_PATCH, \
        $$QT_MAJOR_VERSION, $$QT_MINOR_VERSION, $$QT_PATCH_VERSION))
}

core.subdir = coreSQLiteStudio

tests.subdir = Tests
tests.depends = core

gui.subdir = guiSQLiteStudio
gui.depends = core

cli.subdir = sqlitestudiocli
cli.depends = core

gui_app.subdir = sqlitestudio
gui_app.depends = gui

SUBDIRS += \
    core \
    gui \
    cli \
    gui_app

contains(DEFINES, tests) {
    SUBDIRS += tests
}

OUTPUT_DIR_NAME = output

macx: {
    bundle.commands = sh \"$$PWD/create_macosx_bundle.sh\" \"$$PWD/../$$OUTPUT_DIR_NAME\" \"$$QMAKE_QMAKE\"
    dmg.commands = sh \"$$PWD/create_macosx_bundle.sh\" \"$$PWD/../$$OUTPUT_DIR_NAME\" \"$$QMAKE_QMAKE\" dmg
    pkg.commands = sh \"$$PWD/create_macosx_bundle.sh\" \"$$PWD/../$$OUTPUT_DIR_NAME\" \"$$QMAKE_QMAKE\" dist
    QMAKE_EXTRA_TARGETS += bundle dmg pkg
}

OTHER_FILES += \
    ../ChangeLog.md \
    ../SQLiteStudio-installer.xml
