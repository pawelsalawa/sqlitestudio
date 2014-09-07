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

SUBDIRS += \
    core \
    gui \
    cli \
    UpdateSQLiteStudio \
    gui_app

if(contains(DEFINES,tests)) {
    SUBDIRS += tests
}
