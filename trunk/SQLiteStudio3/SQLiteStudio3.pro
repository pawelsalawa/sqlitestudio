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

win32: {
    SUBDIRS += update_app
}

macx: {
    dest_dir = $$PWD/../output/SQLiteStudio

    clean_bundle.commands = rm -rf $$dest_dir/SQLiteStudio.app/Contents/Frameworks && rm -rf $$dest_dir/SQLiteStudio.app/Contents/PlugIns \
        && rm -f $$dest_dir/SQLiteStudio.app/Contents/MacOS/sqlitestudiocli \
        && rm -f $$dest_dir/SQLiteStudio.app/Contents/Resources/qt.conf

    prepare_dirs.depends = clean_bundle
    prepare_dirs.commands = mkdir $$dest_dir/SQLiteStudio.app/Contents/Frameworks

    copy_plugins.depends = prepare_dirs
    copy_plugins.commands = cp -RP $$dest_dir/plugins $$dest_dir/SQLiteStudio.app/Contents \
        && mv $$dest_dir/SQLiteStudio.app/Contents/plugins $$dest_dir/SQLiteStudio.app/Contents/PlugIns

    copy_app_libs.depends = copy_plugins
    copy_app_libs.commands = cp -RP $$dest_dir/lib*SQLiteStudio*.dylib $$dest_dir/SQLiteStudio.app/Contents/Frameworks

    qtcore_path = $$system("otool -L $$dest_dir/sqlitestudiocli | grep QtCore | awk '{print $1;}'")
    new_qtcore_path = @loader_path/../Frameworks/QtCore.framework/Versions/5/QtCore

    copy_other_apps.depends = copy_app_libs
    copy_other_apps.commands = cp -RP $$dest_dir/sqlitestudiocli $$dest_dir/SQLiteStudio.app/Contents/MacOS \
        && install_name_tool -change libcoreSQLiteStudio.1.dylib @loader_path/../Frameworks/libcoreSQLiteStudio.1.dylib \
            $$dest_dir/SQLiteStudio.app/Contents/MacOS/sqlitestudiocli \
        && install_name_tool -change $$qtcore_path $$new_qtcore_path $$dest_dir/SQLiteStudio.app/Contents/MacOS/sqlitestudiocli

    copy_deps.depends = copy_other_apps
    copy_deps.commands = cp -RP $$PWD/../../lib/*.dylib $$dest_dir/SQLiteStudio.app/Contents/Frameworks

    macdeploy_bin = $$QMAKE_QMAKE
    macdeploy_bin = $$replace(macdeploy_bin, /qmake, /macdeployqt)

    deploy_qt.depends = copy_deps
    deploy_qt.commands = $$macdeploy_bin $$dest_dir/SQLiteStudio.app

    bundle.depends = deploy_qt

    QMAKE_EXTRA_TARGETS += clean_bundle prepare_dirs copy_plugins copy_app_libs copy_other_apps copy_deps deploy_qt bundle
}
