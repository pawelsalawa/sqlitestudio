#!/bin/sh
# shellcheck shell=dash disable=SC2006,SC3003
set -e

printUsage() {
    echo "$0 [-q]... <sqlitestudio build output directory> <Qt path> [dmg|dist|dist_full]"
    echo "You can define DEP_LIB_DIR environment variable pointing to directory with *.dylib files of sqlite3, icu, tcl to use for building the result image."
}

quiet=0
while getopts qu _flag; do
    case "$_flag" in
        q) : $(( quiet += 1 )) ;;
        *) printUsage; exit 1 ;;
    esac
done
shift $(( OPTIND - 1 ))

if [[ -n ${DEBUG_LEVEL+x} ]]; then
    quiet="$DEBUG_LEVEL"
fi

echo "Verbosity level: $quiet"

PYTHON_VERSION="${PYTHON_VERSION:-3.9}"
BACKGROUND_IMG=""  # TODO
BACKGROUND_RGB="56 168 243"

abort() { echo "ERROR: $@"; exit 1; }
debug() { [ "$quiet" -gt 2 ] && echo "DEBUG: $@"; }
info() { [ "$quiet" -gt 1 ] && echo "INFO: $@"; }
run() { [ "$quiet" -gt 0 ] && { printf 'RUN: '; echo "'$@'"; }; "$@"; }

codesign_app() {
    cat > entitlements.plist <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
  <dict>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
    <key>com.apple.security.cs.allow-dyld-environment-variables</key>
    <true/>
  </dict>
</plist>
EOF
    run codesign --force -o runtime --entitlements entitlements.plist --deep --sign - "$1"
    rm entitlements.plist
}

hdiutil_attach() {
    # detach any images with the same name
    mount \
    | awk -v MOUNTPOINT="$2" '$0 ~ MOUNTPOINT {match($1, /disk[0-9]+/); print substr($1, RSTART, RLENGTH)}' \
    | run xargs -tn1 hdiutil detach \
    > /dev/null

    run hdiutil attach "$1" -mountpoint "$2" | awk '/\/dev\// { print $1; exit }'
    sleep 1
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

    _device="$(hdiutil_attach "$_rw_image" "/Volumes/$_volname")"
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

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  printUsage
  exit 1
fi

if [ "$#" -eq 3 ] && [ "$3" != "dmg" ] && [ "$3" != "dist" ] && [ "$3" != "dist_plugins" ] && [ "$3" != "dist_full" ]; then
  printUsage
  exit 1
fi

qt_deploy_bin="$2/bin/macdeployqt"
if [ ! -x "$qt_deploy_bin" ]; then
    abort "$qt_deploy_bin program missing!"
fi
info "macdeployqt executable found: $qt_deploy_bin"

cd "$1/SQLiteStudio" || abort "Could not chdir to $1/SQLiteStudio!"

if [[ -n "${DEP_LIB_DIR:-}" ]]; then
    libdir="$DEP_LIB_DIR"
else
    libdir="$(cd ../../../lib/ && pwd)"
fi
debug "lib:" "$(ls -l "$libdir")"

rm -rf SQLiteStudio.app/Contents/Frameworks
rm -rf SQLiteStudio.app/Contents/PlugIns
rm -f SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
rm -f SQLiteStudio.app/Contents/Resources/qt.conf

mkdir SQLiteStudio.app/Contents/Frameworks

cp -RP plugins SQLiteStudio.app/Contents
mv SQLiteStudio.app/Contents/plugins SQLiteStudio.app/Contents/PlugIns

mkdir -p SQLiteStudio.app/Contents/PlugIns/styles
cp -RP styles/* SQLiteStudio.app/Contents/PlugIns/styles

cp -RP lib/lib*SQLiteStudio*.dylib SQLiteStudio.app/Contents/Frameworks

# Determine our version before any patching, while we have a presumably working binary
export DYLD_FRAMEWORK_PATH=$QT_ROOT_DIR/lib
export DYLD_LIBRARY_PATH=lib:$QT_ROOT_DIR/lib:$libdir
VERSION="$(./sqlitestudiocli -v | awk '{print $2}')"
[ -n "$VERSION" ] || abort "could not determine SQLiteStudio version"

BUILD_ARCHS="$(lipo -archs SQLiteStudio.app/Contents/MacOS/SQLiteStudio)"

# CLI paths
qtcore_path=`otool -L sqlitestudiocli | awk '/QtCore/ {print $1;}'`
new_qtcore_path="@rpath/QtCore.framework/Versions/A/QtCore"

cp -P sqlitestudiocli SQLiteStudio.app/Contents/MacOS
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
install_name_tool -change "$qtcore_path" "$new_qtcore_path" SQLiteStudio.app/Contents/MacOS/sqlitestudiocli

# SQLiteStudio binary paths
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/SQLiteStudio
install_name_tool -change libguiSQLiteStudio.1.dylib "@rpath/libguiSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/SQLiteStudio

# Lib paths
install_name_tool -change libcoreSQLiteStudio.1.dylib "@rpath/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/Frameworks/libguiSQLiteStudio.1.dylib
install_name_tool -change libsqlite3.0.dylib "@rpath/libsqlite3.0.dylib" SQLiteStudio.app/Contents/Frameworks/libcoreSQLiteStudio.1.dylib

embed_libsqlite3() {
    cp -RPf "$libdir/libsqlite3.0.dylib" "$1/Contents/Frameworks"
    ln -sf libsqlite3.0.dylib "$1/Contents/Frameworks/libsqlite3.dylib"
}

embed_libtcl() {
    cp -RPf $libdir/libtcl*.dylib "$1/Contents/Frameworks"
}

debug "in frameworks - 1:" "$(ls -l SQLiteStudio.app/Contents/Frameworks)"
embed_libsqlite3 SQLiteStudio.app
debug "in frameworks - 2:" "$(ls -l SQLiteStudio.app/Contents/Frameworks)"
embed_libtcl SQLiteStudio.app
debug "in frameworks - 3:" "$(ls -l SQLiteStudio.app/Contents/Frameworks)"

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

propose_dylib_changes() {
    local _changes _dest _ref
    otool -L "$1" | awk '{print $1}' | while read -r _ref; do
        case "$_ref" in
            /opt/local/Library/Frameworks/*)   _dest="${_ref#/opt/local/Library/Frameworks/}" ;;
            /opt/local/libexec/openssl3/lib/*) _dest="${_ref#/opt/local/libexec/openssl3/lib/}" ;;
            */libbz2.1.0.dylib | */libexpat.1.dylib | */liblzma.5.dylib | */libz.1.dylib) printf -- "-change %s %s\n" "$_ref" "${_ref##*/}"; continue ;;
            /opt/local/lib/*)                  _dest="${_ref#/opt/local/lib/}" ;;
            *)                                 continue ;;
        esac
        if [ ! -e "$2/$_dest" ]; then
            run cp -pLR "$_ref" "$2/$_dest" || abort "Could not copy $2/$_dest to bundle!";
            run install_name_tool -id "${_dest##/}" "$2/$_dest"
            _changes="$(propose_dylib_changes "$2/$_dest" "$2")"
            # shellcheck disable=SC2086
            [ -z "$_changes" ] || run install_name_tool $_changes "$2/$_dest"
        fi
        printf -- "-change %s @rpath/%s\n" "$_ref" "$_dest"
    done
}

find_local_dependencies() {
    find "$1" -type f -perm +111 -print0 | xargs -0 otool -L \
    | awk '/:$/ { sub(/:$/, ""); f = $1 } /\/(opt|usr)\/local\// { print f, $1 }'
}

assert_no_dylib_problems() {
    local _problems
    _problems="$(find_local_dependencies "$1")"
    # shellcheck disable=SC2086
    [ -z "$_problems" ] || abort 'Unresolved local/ library references:' $_problems
}

deploy_qt() {
    run "$qt_deploy_bin" "$@" -verbose=$((2 - quiet))
}

if [ "$3" = "dmg" ]; then
    deploy_qt SQLiteStudio.app -dmg
elif [ "$3" = "dist" ]; then
    deploy_qt SQLiteStudio.app -executable=SQLiteStudio.app/Contents/MacOS/SQLiteStudio

    # Fix sqlite3 file in the image
    embed_libsqlite3 SQLiteStudio.app
    
    # Same for Tcl
    embed_libtcl SQLiteStudio.app

    # Fix python dependencies in the image if linked to a Python library
    python_plugin_lib="SQLiteStudio.app/Contents/PlugIns/libScriptingPython.dylib"
    run install_name_tool -change "@loader_path/../Frameworks/libpython$PYTHON_VERSION.dylib" "libpython$PYTHON_VERSION.dylib" "$python_plugin_lib"

    # Fix other dependencies which can be supplied by system
    find_local_dependencies SQLiteStudio.app | while read -r _binary _ref; do
        case "$_ref" in
            */libbz2.1.0.dylib | */libexpat.1.dylib | */liblzma.5.dylib | */libz.1.dylib) ls -l "$_binary"; run install_name_tool -change "$_ref" "${_ref##*/}" "$_binary" ;;
        esac
    done

    codesign_app "SQLiteStudio.app"

    assert_no_dylib_problems SQLiteStudio.app

    VERSION=`SQLiteStudio.app/Contents/MacOS/sqlitestudiocli -v | awk '{print $2}'`
    [ -n "$VERSION" ] || abort "could not determine SQLiteStudio version"

    ls -l
    # shellcheck disable=SC2086
    pretty_dmg "SQLiteStudio.app" "SQLiteStudio-$VERSION" "$BACKGROUND_IMG" $BACKGROUND_RGB

    rm -fr thinned
    ls -l -- *.dmg
    info "Done."
else
    deploy_qt SQLiteStudio.app
fi
