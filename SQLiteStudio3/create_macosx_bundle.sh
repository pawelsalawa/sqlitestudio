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

hdiutil_create() {
    run hdiutil create \
        -fs HFS+ -fsargs '-c c=64,a=16,e=16' \
        -scrub \
        "$@"
}

pretty_dmg() {
    local _appname="${1%.app}" _volname="$2" _image_path="$3" _rgb_16bit
    [ -z "$6" ] || _rgb_16bit="{$(( $4 * 257 )), $(( $5 * 257 )), $(( $6 * 257 ))}"
    local _rw_image="$_volname-rw.dmg.sparseimage" _device
    [ ! -f "$_rw_image" ] || run rm -f "$_rw_image"
    hdiutil_create \
        -format UDSP \
        -srcfolder "$_appname.app" \
        -size "$(du -ms "$_appname.app" | awk '{ print (2 ^ int(log($1) / log(2) + 2.5)) "m" }')" \
        -volname "$_volname" \
        "$_rw_image"

    # detach any images with the same name
    mount \
    | awk '/\/Volumes\/'"$_volname"' / {match($1, /disk[0-9]+/); print substr($1, RSTART, RLENGTH)}' \
    | run xargs -tn1 hdiutil detach

    _device="$(run hdiutil attach "$_rw_image" | awk '/\/dev\// { print $1; exit }')"
    sleep 1
    if [ -n "$_image_path" ]; then
        run mkdir "/Volumes/$_volname/.background"
        run cp "$_image_path" "/Volumes/$_volname/.background/"
    fi
    run osascript <<EOF
        tell application "Finder"
            tell disk "$_volname"
                open
                set current view of container window to icon view
                set toolbar visible of container window to false
                set statusbar visible of container window to false
                set the bounds of container window to {400, 100, 885, 430}
                set theViewOptions to the icon view options of container window
                set arrangement of theViewOptions to not arranged
                set icon size of theViewOptions to 72
                $([ -z "$_image_path" ] || echo "set background picture of theViewOptions to file \".background:$(basename "$3")\"")
                $([ -z "$_rgb_16bit" ] || echo "set background color of theViewOptions to $_rgb_16bit")
                make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
                set position of item "$_appname" of container window to {100, 100}
                set position of item "Applications" of container window to {375, 100}
                update without registering applications
                delay 3
                close
            end tell
        end tell
EOF
    run hdiutil detach "$_device"
    run hdiutil compact "$_rw_image" -batteryallowed
    [ ! -f "$_volname.dmg" ] || run rm -f "$_volname.dmg"
    run hdiutil convert "$_rw_image" -format ULFO -o "$_volname.dmg"
    run rm "$_rw_image"
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

    run "$qt_deploy_bin" SQLiteStudio.app -executable=SQLiteStudio.app/Contents/MacOS/SQLiteStudio -verbose=$((2 - quiet))

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

    assert_no_dylib_problems SQLiteStudio.app

    VERSION=`SQLiteStudio.app/Contents/MacOS/sqlitestudiocli -v | awk '{print $2}'`
    [ -n "$VERSION" ] || abort "could not determine SQLiteStudio version"

    ls -l
    _background_img=""  # TODO
    _background_rgb="56 168 243"
    # shellcheck disable=SC2086
    pretty_dmg "SQLiteStudio.app" "SQLiteStudio-$VERSION" "$_background_img" $_background_rgb

    ls -l -- *.dmg
    info "Done."
else
    "$qt_deploy_bin" SQLiteStudio.app
    replaceInfo "$1"
fi
