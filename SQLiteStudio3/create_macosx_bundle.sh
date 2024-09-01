#!/bin/sh
# shellcheck shell=dash disable=SC2006,SC3003
set -e

printUsage() {
    echo "$0 [-q]... <sqlitestudio build output directory> <qmake path> [dmg|dist|dist_full]"
    echo "$0 -u <sqlitestudio.x86_64.dmg> <sqlitestudio.arm64.dmg> <sqlitestudio.universal.dmg>"
}

quiet=0
universalize=0
while getopts qu _flag; do
    case "$_flag" in
        q) : $(( quiet += 1 )) ;;
        u) : $(( universalize += 1 )) ;;
        *) printUsage; exit 1 ;;
    esac
done
shift $(( OPTIND - 1 ))

PYTHON_VERSION="${PYTHON_VERSION:-3.9}"
BACKGROUND_IMG=""  # TODO
BACKGROUND_RGB="56 168 243"

abort() { printf "ERROR: %s\n" "$@" 1>&2; exit 1; }
debug() { [ "$quiet" -gt 0 ] || printf "DEBUG: %s\n" "$@" 1>&2; }
info() { [ "$quiet" -gt 1 ] || printf "INFO: %s\n" "$@" 1>&2; }
run() { [ "$quiet" -gt 2 ] || { printf 'RUN: '; printf "'%s' " "$@"; printf '\n'; } 1>&2; "$@"; }

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

universalize() {
    _mountpoint1=/Volumes/sqlitestudio-arch1
    _mountpoint2=/Volumes/sqlitestudio-arch2
    _device1="$(hdiutil_attach "$1" "$_mountpoint1")"
    _device2="$(hdiutil_attach "$2" "$_mountpoint2")"
    rm -fr _universalized
    cp -RP "$_mountpoint1" _universalized
    find "$_mountpoint1" -type f \( -perm +u+x -or -name '*.dylib' \) | while read -r _name1; do
        case "$(file -b "$_name1")" in Mach-O*)
            _relative_name="${_name1#"$_mountpoint1/"}"
            run lipo "$_name1" "$_mountpoint2/$_relative_name" -create -output "_universalized/$_relative_name"
            ;;
        esac
    done
    run hdiutil detach "$_device1"
    run hdiutil detach "$_device2"
    cd _universalized
    codesign_app "SQLiteStudio.app"
    # shellcheck disable=SC2086
    pretty_dmg "SQLiteStudio.app" "universal_out" "$BACKGROUND_IMG" $BACKGROUND_RGB
    cd ..
    mv "_universalized/universal_out.dmg" "$3"
    rm -fr _universalized
}

if [ "$universalize" -gt 0 ] && [ -f "$1" ] && [ -f "$2" ] && [ -n "$3" ]; then
    universalize "$@"
    exit
fi

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  printUsage
  exit 1
fi

if [ "$#" -eq 3 ] && [ "$3" != "dmg" ] && [ "$3" != "dist" ] && [ "$3" != "dist_plugins" ] && [ "$3" != "dist_full" ]; then
  printUsage
  exit 1
fi

qt_version="$("$2" -v | awk '/Qt version/ { print $4 }')"
info "Qt version detected: $qt_version"
case "$qt_version" in
  5*) qt_version_path=5 ;;
  6*) qt_version_path=A ;;
esac
qmake_basename="${2##*/}"
qt_deploy_bin="$(echo "$2" | sed "s/$qmake_basename\$/macdeployqt/")"
info "macdeployqt executable found: $qt_deploy_bin"
if [ ! -x "$qt_deploy_bin" ]; then
    abort "$qt_deploy_bin program missing!"
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

BUILD_ARCHS="$(lipo -archs SQLiteStudio.app/Contents/MacOS/SQLiteStudio)"

# CLI paths
qtcore_path=`otool -L sqlitestudiocli | awk '/QtCore/ {print $1;}'`
new_qtcore_path="@rpath/QtCore.framework/Versions/$qt_version_path/QtCore"

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

embed_python_framework() (
    local _src_framework="$1" _ver="$2" _app="$3"
    local _dest_framework="$_app/Contents/Frameworks/Python.framework"
    run mkdir -p "$_dest_framework/Versions"
    run cd "$_dest_framework"
    run cp -RP "$_src_framework/Versions/$PYTHON_VERSION" Versions/
    run rm -fr "Versions/$PYTHON_VERSION/lib/python$PYTHON_VERSION/idlelib"
    run rm -fr "Versions/$PYTHON_VERSION/lib/python$PYTHON_VERSION/test"
    run ln -s "$PYTHON_VERSION" Versions/Current
    run ln -s Versions/Current/Headers Versions/Current/Python Versions/Current/Resources .
    run install_name_tool -id "@executable_path/../Frameworks/Python.framework/Versions/$PYTHON_VERSION/Python" "Versions/$PYTHON_VERSION/Python"

    # In each executable, apply /opt/local/lib changes
    find "Versions/$PYTHON_VERSION" -type f -perm +111 | while read -r _filename; do
        if [ "$_filename" = "Versions/$PYTHON_VERSION/Resources/Python.app/Contents/MacOS/Python" ]; then
            _changes="-change /opt/local/lib/libintl.8.dylib @loader_path/../../../../../../../libintl.8.dylib
-change $_src_framework/Versions/$PYTHON_VERSION/Python @loader_path/../../../../Python"
        else
            _changes="$(propose_dylib_changes "$_filename" ..)"
        fi
        # shellcheck disable=SC2086
        [ -z "$_changes" ] || run install_name_tool $_changes "$_filename"
    done
)

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

thin_app() {
    # Given a path, thin all universal binaries there to ARM64-only and
    # codesign them with an ad-hoc signature
    find "$1" -type f -perm +111 -size +2100c | grep -Ev '\.py$|install-sh$|makesetup$|fetch_macholib$' \
    | while read -r _binary; do
        case "$(lipo -archs "$_binary")" in
            x86_64) run rm "$_binary" ;;
            *64" "*64)
                run lipo -thin arm64 "$_binary" -output "$_binary.arm64"
                run mv "$_binary.arm64" "$_binary"
                ;;
        esac
    done
    codesign_app "$1"
}

thin_dmg() (
    thin_app "$1"
    cd "$(dirname "$1")"
    # shellcheck disable=SC2086
    pretty_dmg "$(basename "$1")" "$2" "$BACKGROUND_IMG" $BACKGROUND_RGB
    mv "$2.dmg" "../$2-arm64.dmg"
)

deploy_qt() {
    run "$qt_deploy_bin" "$@" -verbose=$((2 - quiet))
}

if [ "$3" = "dmg" ]; then
    replaceInfo "$1"
    deploy_qt SQLiteStudio.app -dmg
elif [ "$3" = "dist" ]; then
    replaceInfo "$1"

    deploy_qt SQLiteStudio.app -executable=SQLiteStudio.app/Contents/MacOS/SQLiteStudio

    # Fix sqlite3 file in the image
    embed_libsqlite3 SQLiteStudio.app

    # Fix python dependencies in the image if linked to a Python library
    python_plugin_lib="SQLiteStudio.app/Contents/PlugIns/libScriptingPython.dylib"
    if otool -L "$python_plugin_lib" | grep -q /opt/local/Library/Frameworks/Python.framework; then
        python_from_macports="yes"
        _ref="/opt/local/Library/Frameworks/Python.framework/Versions/$PYTHON_VERSION/Python"
    else
        python_from_macports="no"
        run rm -f SQLiteStudio.app/Contents/Frameworks/libpython*
        _ref="@loader_path/../Frameworks/libpython$PYTHON_VERSION.dylib"
    fi
    run install_name_tool -change "$_ref" "libpython$PYTHON_VERSION.dylib" "$python_plugin_lib"

    # Fix other dependencies which can be supplied by system
    find_local_dependencies SQLiteStudio.app | while read -r _binary _ref; do
        case "$_ref" in
            */libbz2.1.0.dylib | */libexpat.1.dylib | */liblzma.5.dylib | */libz.1.dylib) ls -l "$_binary"; run install_name_tool -change "$_ref" "${_ref##*/}" "$_binary" ;;
        esac
    done

    case "$BUILD_ARCHS" in arm64) codesign_app "SQLiteStudio.app" ;; esac

    assert_no_dylib_problems SQLiteStudio.app

    VERSION=`SQLiteStudio.app/Contents/MacOS/sqlitestudiocli -v | awk '{print $2}'`
    [ -n "$VERSION" ] || abort "could not determine SQLiteStudio version"

    ls -l
    # shellcheck disable=SC2086
    pretty_dmg "SQLiteStudio.app" "SQLiteStudio-$VERSION" "$BACKGROUND_IMG" $BACKGROUND_RGB

    case "$BUILD_ARCHS" in *64" "*64)
        info "Universal build detected. Making an ARM64-only image"
        mkdir -p thinned
        cp -RPc "SQLiteStudio.app" thinned/
        thin_dmg "thinned/SQLiteStudio.app" "SQLiteStudio-$VERSION"
        ;;
    esac

    if [ "$python_from_macports" = "yes" ]; then
        info "MacPorts Python detected. Making an image with bundled Python"

        embed_python_framework /opt/local/Library/Frameworks/Python.framework "$PYTHON_VERSION" SQLiteStudio.app

        run install_name_tool \
            -change "libpython$PYTHON_VERSION.dylib" "@loader_path/../Frameworks/Python.framework/Versions/$PYTHON_VERSION/Python" \
            "$python_plugin_lib"

        case "$BUILD_ARCHS" in arm64) codesign_app "SQLiteStudio.app" ;; esac

        assert_no_dylib_problems SQLiteStudio.app

        # shellcheck disable=SC2086
        pretty_dmg "SQLiteStudio.app" "SQLiteStudio-$VERSION-py$PYTHON_VERSION" "$BACKGROUND_IMG" $BACKGROUND_RGB

        run rm -fr SQLiteStudio.app/Contents/Frameworks/Python.framework
        run install_name_tool \
            -change "@loader_path/../Frameworks/Python.framework/Versions/$PYTHON_VERSION/Python" "libpython$PYTHON_VERSION.dylib" \
            "$python_plugin_lib"

        case "$BUILD_ARCHS" in *64" "*64)
            info "Universal build detected. Making an ARM64-only image with Python"
            embed_python_framework /opt/local/Library/Frameworks/Python.framework "$PYTHON_VERSION" thinned/SQLiteStudio.app
            run install_name_tool \
                -change "libpython$PYTHON_VERSION.dylib" "@loader_path/../Frameworks/Python.framework/Versions/$PYTHON_VERSION/Python" \
                "thinned/$python_plugin_lib"
            thin_dmg "thinned/SQLiteStudio.app" "SQLiteStudio-$VERSION-py$PYTHON_VERSION"
            ;;
        esac
    fi
    rm -fr thinned
    ls -l -- *.dmg
    info "Done."
else
    deploy_qt SQLiteStudio.app
    replaceInfo "$1"
fi
