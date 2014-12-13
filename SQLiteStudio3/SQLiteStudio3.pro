TEMPLATE = subdirs

core.subdir = coreSQLiteStudio

tests.subdir = Tests
tests.depends = core

gui.subdir = guiSQLiteStudio
gui.depends = core

cli.subdir = sqlitestudiocli
cli.depends = core

gui_app.subdir = sqlitestudio
gui_app.depends = gui

update_app.subdir = UpdateSQLiteStudio
update_app.depends = core

SUBDIRS += \
    core \
    gui \
    cli \
    gui_app

if(contains(DEFINES,tests)) {
    SUBDIRS += tests
}

OUTPUT_DIR_NAME = output

win32: {
    SUBDIRS += update_app
}

linux: {
    portable.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE
    tgz.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE tgz
    dist.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist
    QMAKE_EXTRA_TARGETS += portable tgz dist
}

macx: {
    bundle.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE
    dmg.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dmg
    dist.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist
    QMAKE_EXTRA_TARGETS += bundle dmg dist
}
