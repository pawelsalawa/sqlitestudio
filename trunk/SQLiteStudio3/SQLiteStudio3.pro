TEMPLATE = subdirs

core.subdir = coreSQLiteStudio

tests.subdir = Tests
tests.depends = core

gui.subdir = SQLiteStudio
gui.depends = core

cli.subdir = Console
cli.depends = core

SUBDIRS += \
    core \
    gui \
    cli

if(contains(DEFINES,tests)) {
    SUBDIRS += tests
}
