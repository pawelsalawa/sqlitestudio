#!/bin/sh
# shellcheck shell=dash disable=SC2006,SC3003
set -e

printUsage() {
    echo "$0 [-q]... <sqlitestudio build output directory> <qmake path> [dmg|dist|dist_full]"
}

quiet=0
while getopts q _flag; do
    case "$_flag" in
        q) : $(( quiet += 1 )) ;;
        *) printUsage; exit 1 ;;
    esac
done
shift $(( OPTIND - 1 ))

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  printUsage
  exit 1
fi

if [ "$#" -eq 3 ] && [ "$3" != "dmg" ] && [ "$3" != "dist" ] && [ "$3" != "dist_plugins" ] && [ "$3" != "dist_full" ]; then
  printUsage
  exit 1
fi

abort() { printf "ERROR: %s\n" "$@" 1>&2; exit 1; }
debug() { [ "$quiet" -gt 0 ] || printf "DEBUG: %s\n" "$@" 1>&2; }
info() { [ "$quiet" -gt 1 ] || printf "INFO: %s\n" "$@" 1>&2; }
run() { [ "$quiet" -gt 2 ] || { printf 'RUN: '; printf "'%s' " "$@"; printf '\n'; } 1>&2; "$@"; }

PYTHON_VERSION="${PYTHON_VERSION:-3.9}"

qt_deploy_bin="$(echo "$2" | sed 's/qmake$/macdeployqt/')"
if [ ! -x "$qt_deploy_bin" ]; then
    abort "macdeployqt program missing!"
fi

cd "$1/SQLiteStudio" || abort "Could not chdir to $1/SQLiteStudio!"

rm -rf SQLiteStudio.app/Contents/Frameworks
rm -rf SQLiteStudio.app/Contents/PlugIns
rm -f SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
rm -f SQLiteStudio.app/Contents/Resources/qt.conf

mkdir SQLiteStudio.app/Contents/Frameworks

cp -RP plugins SQLiteStudio.app/Contents
mv SQLiteStudio.app/Contents/plugins SQLiteStudio.app/Contents/PlugIns

mkdir -p SQLiteStudio.app/Contents/PlugIns/styles
cp -RP styles/* SQLiteStudio.app/Contents/PlugIns/styles

cp -RP lib*SQLiteStudio*.dylib SQLiteStudio.app/Contents/Frameworks

# Determine our version before any patching, while we have a presumably working binary
VERSION="$(./sqlitestudiocli -v | awk '{print $2}')"
[ -n "$VERSION" ] || abort "could not determine SQLiteStudio version"

# CLI paths
qtcore_path=`otool -L sqlitestudiocli | awk '/QtCore/ {print $1;}'`
new_qtcore_path="@rpath/QtCore.framework/Versions/5/QtCore"

cp -P sqlitestudiocli SQLiteStudio.app/Contents/MacOS
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
install_name_tool -change "$qtcore_path" "$new_qtcore_path" SQLiteStudio.app/Contents/MacOS/sqlitestudiocli

# SQLiteStudio binary paths
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/SQLiteStudio
install_name_tool -change libguiSQLiteStudio.1.dylib "@rpath/libguiSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/SQLiteStudio

# Lib paths
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/Frameworks/libguiSQLiteStudio.1.dylib
install_name_tool -change libsqlite3.0.dylib "@rpath/libsqlite3.0.dylib" SQLiteStudio.app/Contents/Frameworks/libcoreSQLiteStudio.1.dylib

libdir=$(cd ../../../lib/ && pwd)
debug "lib:" "$(ls -l "$libdir")"

debug "in frameworks - 1:" "$(ls -l SQLiteStudio.app/Contents/Frameworks)"

embed_libsqlite3() {
    cp -RPf "$libdir/libsqlite3.0.dylib" "$1/Contents/Frameworks"
    libsqlite3_in_bundle="$1/Contents/Frameworks/libsqlite3.0.dylib"
    ln -sf libsqlite3.0.dylib "$1/Contents/Frameworks/libsqlite3.dylib"
    if otool -L "$libsqlite3_in_bundle" | grep -q /opt/local; then
        info "MacPorts libsqlite3.0.dylib detected! Fixing dylib references"
        run install_name_tool \
          -change /opt/local/lib/libz.1.dylib "@rpath/libz.1.dylib" \
          -id "@executable_path/../Frameworks/libsqlite3.0.dylib" \
          "$libsqlite3_in_bundle"
    fi
}
embed_libsqlite3 SQLiteStudio.app

debug "in frameworks - 2:" "$(ls -l SQLiteStudio.app/Contents/Frameworks)"

# Plugin paths
fixPluginPaths() {
    for PLUGIN_FILE in "$1"/*; do
        if [ -f "$PLUGIN_FILE" ]; then
    	    info "Fixing paths for plugin $PLUGIN_FILE"
            install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" "$PLUGIN_FILE"
            install_name_tool -change libguiSQLiteStudio.1.dylib "@rpath/libguiSQLiteStudio.1.dylib" "$PLUGIN_FILE"
        fi
        if [ -d "$PLUGIN_FILE" ]; then
            fixPluginPaths "$PLUGIN_FILE"
        fi
    done
}
fixPluginPaths SQLiteStudio.app/Contents/PlugIns

replaceInfo() {
    local _contents="$1/SQLiteStudio/SQLiteStudio.app/Contents"
    info "Replacing Info.plist"
    YEAR=`date '+%Y'`

    run sed -e "s/%VERSION%/$VERSION/g" -e "s/%YEAR%/$YEAR/g" "$_contents/Info.plist" > "$_contents/Info.plist.new"
    debug "New plist:" "$(cat "$_contents/Info.plist.new")"
    run mv "$_contents/Info.plist.new" "$_contents/Info.plist"
}

find_local_dependencies() {
    find "$1" -type f -perm +111 -print0 | xargs -0 otool -L \
    | awk '/:$/ { sub(/:$/, ""); f = $1 } /\/(opt|usr)\/local\// { print f, $1 }'
}


if [ "$3" = "dmg" ]; then
    replaceInfo "$1"
    "$qt_deploy_bin" SQLiteStudio.app -dmg
elif [ "$3" = "dist" ]; then
    replaceInfo "$1"

    run "$qt_deploy_bin" SQLiteStudio.app -dmg -executable=SQLiteStudio.app/Contents/MacOS/SQLiteStudio -verbose=$((2 - quiet))

	cd "$1/SQLiteStudio"

	mv SQLiteStudio.dmg "sqlitestudio-$VERSION.dmg"
	hdiutil attach "sqlitestudio-$VERSION.dmg"

	hdiutil detach /Volumes/SQLiteStudio

    # Convert image to RW and attach	
	hdiutil convert "sqlitestudio-$VERSION.dmg" -format UDRW -o "sqlitestudio-rw-$VERSION.dmg"
	hdiutil attach -readwrite "sqlitestudio-rw-$VERSION.dmg"
    cd /Volumes/SQLiteStudio

    # Fix sqlite3 file in the image
    embed_libsqlite3 SQLiteStudio.app

    # Fix python dependencies in the image
    python_plugin_lib="SQLiteStudio.app/Contents/PlugIns/libScriptingPython.dylib"
    if otool -L "$python_plugin_lib" | grep -q /opt/local/Library/Frameworks/Python.framework; then
        python_from_macports="yes"
        _ref="/opt/local/Library/Frameworks/Python.framework/Versions/$PYTHON_VERSION/Python"
    else
        run rm -f SQLiteStudio.app/Contents/Frameworks/libpython* SQLiteStudio.app/Contents/Frameworks/libint*
        _ref="@loader_path/../Frameworks/libpython$PYTHON_VERSION.dylib"
    fi
    run install_name_tool -change "$_ref" "libpython$PYTHON_VERSION.dylib" "$python_plugin_lib"

    # Fix other dependencies which can be supplied by system
    find_local_dependencies SQLiteStudio.app | while read -r _binary _ref; do
        case "$_ref" in
            */libbz2.1.0.dylib | */libexpat.1.dylib | */liblzma.5.dylib | */libz.1.dylib) ls -l "$_binary"; run install_name_tool -change "$_ref" "${_ref##*/}" "$_binary" ;;
        esac
    done

	# Detach RW image
	hdiutil detach /Volumes/SQLiteStudio
	hdiutil compact "sqlitestudio-rw-$VERSION.dmg"
	
	# Convert image back to RO and compressed
	rm -f "sqlitestudio-$VERSION.dmg"
	hdiutil convert "sqlitestudio-rw-$VERSION.dmg" -format UDZO -o "sqlitestudio-$VERSION.dmg"
	rm -f "sqlitestudio-rw-$VERSION.dmg"
	
	echo "Verifying contents of new image:"
	hdiutil attach "sqlitestudio-$VERSION.dmg"
	ls -l /Volumes/SQLiteStudio/SQLiteStudio.app/Contents/Frameworks
	hdiutil detach /Volumes/SQLiteStudio
	
    info "Done."
else
    "$qt_deploy_bin" SQLiteStudio.app
    replaceInfo "$1"
fi
