#!/usr/bin/env bash
# shellcheck shell=dash disable=SC2006,SC3003
set -e

printUsage() {
    echo "$0 [-q]... <letos build output directory> <Qt path> [dmg|dist|dist_full]"
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

ORIGINAL_DIR=$(pwd)
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

PYTHON_VERSION="${PYTHON_VERSION:-3.9}"
BACKGROUND_IMG=""  # TODO
BACKGROUND_RGB="56 168 243"

cleanup() {
    cd "$ORIGINAL_DIR"
}

trap cleanup EXIT INT TERM

abort() { echo "ERROR: $@"; exit 1; }
debug() { [ "$quiet" -gt 2 ] && echo "DEBUG: $@"; }
info() { [ "$quiet" -gt 1 ] && echo "INFO: $@"; }
run() { [ "$quiet" -gt 0 ] && { printf 'RUN: '; echo "'$@'"; }; "$@"; }

SIGN_MODE="${SIGN_MODE:-adhoc}"
CODESIGN_IDENTITY="${CODESIGN_IDENTITY:--}"

if [ "$SIGN_MODE" = "developer-id" ]; then
    CODESIGN_IDENTITY="${CODESIGN_IDENTITY:?CODESIGN_IDENTITY is required}"
elif [ "$SIGN_MODE" = "adhoc" ]; then
    CODESIGN_IDENTITY="-"
else
    abort "Unknown SIGN_MODE: $SIGN_MODE"
fi

NOTARY_PROFILE="${NOTARY_PROFILE:-letos-notary}"

codesign_app() {
    if [ "$SIGN_MODE" = "developer-id" ]; then

        # Signing with Dev ID
        cat > entitlements.plist <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<plist version="1.0">
<dict>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
</dict>
</plist>
EOF
        SIGN_DIRS=()
        [ -d "$1/Contents/Frameworks" ] && SIGN_DIRS+=("$1/Contents/Frameworks")
        [ -d "$1/Contents/PlugIns" ] && SIGN_DIRS+=("$1/Contents/PlugIns")
        [ -d "$1/Contents/extensions" ] && SIGN_DIRS+=("$1/Contents/extensions")

        find "${SIGN_DIRS[@]}" \
            -type f \( -name "*.dylib" -o -perm +111 \) \
            -print0 | while IFS= read -r -d '' file; do
                run codesign --force --timestamp --options runtime \
                    --sign "$CODESIGN_IDENTITY" "$file"
            done

        run codesign --force --timestamp --entitlements entitlements.plist --options runtime \
            --sign "$CODESIGN_IDENTITY" \
            "$1/Contents/MacOS/Letos"

        run codesign --force --timestamp --entitlements entitlements.plist --options runtime \
            --sign "$CODESIGN_IDENTITY" \
            "$1"

        run codesign --verify --strict --deep --verbose=4 "$1"

     else

        # Self-signed for local dev env
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

     fi
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

cd "$1/Letos" || abort "Could not chdir to $1/Letos!"

if [[ -n "${DEP_LIB_DIR:-}" ]]; then
    libdir="$DEP_LIB_DIR"
else
    libdir="$(cd ../../../lib/ && pwd)"
fi
debug "lib:" "$(ls -l "$libdir")"

rm -rf Letos.app/Contents/Frameworks
rm -rf Letos.app/Contents/PlugIns
rm -f Letos.app/Contents/Resources/qt.conf

mkdir Letos.app/Contents/Frameworks

cp -RP plugins Letos.app/Contents
mv Letos.app/Contents/plugins Letos.app/Contents/PlugIns

mkdir -p Letos.app/Contents/PlugIns/styles
cp -RP styles/* Letos.app/Contents/PlugIns/styles

cp -RP lib/lib*Letos*.dylib Letos.app/Contents/Frameworks

# Determine our version before any patching, while we have a presumably working binary
export DYLD_FRAMEWORK_PATH=$QT_ROOT_DIR/lib
export DYLD_LIBRARY_PATH=lib:$QT_ROOT_DIR/lib:$libdir
VERSION="$($SCRIPT_DIR/../version_from_source.sh)"
[ -n "$VERSION" ] || abort "could not determine Letos version"

BUILD_ARCHS="$(lipo -archs Letos.app/Contents/MacOS/Letos)"

# Letos binary paths
install_name_tool -change libcoreLetos.1.dylib "@rpath/libcoreLetos.1.dylib" Letos.app/Contents/MacOS/Letos
install_name_tool -change libguiLetos.1.dylib "@rpath/libguiLetos.1.dylib" Letos.app/Contents/MacOS/Letos
install_name_tool -change libsqlite3.0.dylib "@rpath/libsqlite3.0.dylib" Letos.app/Contents/MacOS/Letos

# Lib paths
install_name_tool -change libcoreLetos.1.dylib "@rpath/libcoreLetos.1.dylib" Letos.app/Contents/Frameworks/libguiLetos.1.dylib
install_name_tool -change libsqlite3.0.dylib "@rpath/libsqlite3.0.dylib" Letos.app/Contents/Frameworks/libcoreLetos.1.dylib

embed_libsqlite3() {
    cp -RPf "$libdir/libsqlite3.0.dylib" "$1/Contents/Frameworks"
    ln -sf libsqlite3.0.dylib "$1/Contents/Frameworks/libsqlite3.dylib"
}

embed_libtcl() {
    cp -RPf $libdir/libtcl*.dylib "$1/Contents/Frameworks"
}

debug "in frameworks - 1:" "$(ls -l Letos.app/Contents/Frameworks)"
embed_libsqlite3 Letos.app
debug "in frameworks - 2:" "$(ls -l Letos.app/Contents/Frameworks)"
embed_libtcl Letos.app
debug "in frameworks - 3:" "$(ls -l Letos.app/Contents/Frameworks)"

# Plugin paths
fixPluginPaths() {
    for PLUGIN_FILE in "$1"/*; do
        if [ -f "$PLUGIN_FILE" ]; then
            info "Fixing paths for plugin $PLUGIN_FILE"
            install_name_tool -change libcoreLetos.1.dylib "@rpath/libcoreLetos.1.dylib" "$PLUGIN_FILE"
            install_name_tool -change libguiLetos.1.dylib "@rpath/libguiLetos.1.dylib" "$PLUGIN_FILE"
        fi
        if [ -d "$PLUGIN_FILE" ]; then
            fixPluginPaths "$PLUGIN_FILE"
        fi
    done
}
fixPluginPaths Letos.app/Contents/PlugIns

fixSqlitePath() {
    find "$1" -type f \( -name "*.dylib" -o -perm +111 \) -print0 |
    while IFS= read -r -d '' f; do
        if otool -L "$f" 2>/dev/null | grep -q '^[[:space:]]*libsqlite3\.0\.dylib'; then
            run install_name_tool -change libsqlite3.0.dylib "@rpath/libsqlite3.0.dylib" "$f"
        fi
    done
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
    deploy_qt Letos.app -dmg
elif [ "$3" = "dist" ]; then
    deploy_qt Letos.app -executable=Letos.app/Contents/MacOS/Letos

    # Fix executable's RPATH
    run install_name_tool -add_rpath @executable_path/../Frameworks Letos.app/Contents/MacOS/Letos

    # Fix sqlite3 file in the image
    embed_libsqlite3 Letos.app
    fixSqlitePath Letos.app
    
    # Same for Tcl
    embed_libtcl Letos.app

    # Fix python dependencies in the image if linked to a Python library
    python_plugin_lib="Letos.app/Contents/PlugIns/libScriptingPython.dylib"
    run install_name_tool -change "@loader_path/../Frameworks/libpython$PYTHON_VERSION.dylib" "libpython$PYTHON_VERSION.dylib" "$python_plugin_lib"

    # Fix other dependencies which can be supplied by system
    find_local_dependencies Letos.app | while read -r _binary _ref; do
        case "$_ref" in
            */libbz2.1.0.dylib | */libexpat.1.dylib | */liblzma.5.dylib | */libz.1.dylib) ls -l "$_binary"; run install_name_tool -change "$_ref" "${_ref##*/}" "$_binary" ;;
        esac
    done

    codesign_app "Letos.app"

    assert_no_dylib_problems Letos.app

    [ -n "$VERSION" ] || abort "could not determine Letos version"

    ls -l
    # shellcheck disable=SC2086
    pretty_dmg "Letos.app" "Letos-$VERSION" "$BACKGROUND_IMG" $BACKGROUND_RGB

    #  Sign & Notary
    DMG_FILE="Letos-$VERSION.dmg"
    if [ "$SIGN_MODE" = "developer-id" ]; then
        run codesign --force --timestamp --sign "$CODESIGN_IDENTITY" "$DMG_FILE"
        NOTARY_JSON="$(xcrun notarytool submit "$DMG_FILE" --keychain-profile "$NOTARY_PROFILE" --output-format json)"

        echo "$NOTARY_JSON"

        NOTARY_ID="$(echo "$NOTARY_JSON" | jq -r '.id // empty')"

        if [ -z "$NOTARY_ID" ]; then
            abort "Could not extract notarization ID"
        fi

        NOTARY_ACCEPTED=false

        for i in {1..60}; do
            if ! INFO_JSON="$(xcrun notarytool info "$NOTARY_ID" --keychain-profile "$NOTARY_PROFILE" --output-format json)"; then
                echo "Could not query notarization status, retrying..."
                sleep 30
                continue
            fi

            echo "$INFO_JSON"

            STATUS="$(echo "$INFO_JSON" | jq -r '.status // empty')"

            case "$STATUS" in
                Accepted)
                    NOTARY_ACCEPTED=true
                    break
                    ;;
                Invalid|Rejected)
                    xcrun notarytool log "$NOTARY_ID" --keychain-profile "$NOTARY_PROFILE"
                    abort "Notarization failed: $STATUS"
                    ;;
            esac

            sleep 30
        done

        if [ "$NOTARY_ACCEPTED" != "true" ]; then
            xcrun notarytool log "$NOTARY_ID" --keychain-profile "$NOTARY_PROFILE" || true
            abort "Notarization timed out"
        fi

        STAPLE_RETRY_DELAYS=(15 30 60 120)
        STAPLE_ATTEMPT=0
        while true; do
            if run xcrun stapler staple --verbose "$DMG_FILE"; then
                break
            else
                STAPLE_STATUS=$?
            fi

            if [ "$STAPLE_STATUS" -ne 68 ] || [ "$STAPLE_ATTEMPT" -ge "${#STAPLE_RETRY_DELAYS[@]}" ]; then
                exit "$STAPLE_STATUS"
            fi

            STAPLE_DELAY="${STAPLE_RETRY_DELAYS[$STAPLE_ATTEMPT]}"
            echo "Stapling failed with error 68; retrying in $STAPLE_DELAY seconds..."
            sleep "$STAPLE_DELAY"
            STAPLE_ATTEMPT=$((STAPLE_ATTEMPT + 1))
        done
        run xcrun stapler validate "$DMG_FILE"
    fi

    rm -fr thinned
    ls -l -- *.dmg
    info "Done."
else
    deploy_qt Letos.app
fi
