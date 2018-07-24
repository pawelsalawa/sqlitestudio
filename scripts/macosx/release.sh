#!/bin/sh

TARGET_DIR=/tmp/sqls-installer
QMAKE=~/Qt/5.10.0/clang_64/bin/qmake
TMP_DIR=~/tmp/SQLiteStudio.app
CORES=3
RECOMPILE=1

if [ ! -f $QMAKE ]; then
    echo "qmake at $QMAKE does not exist."
    exit 1
fi

OLDDIR=`pwd`

if [ "$RECOMPILE" != "0" ]; then
    rm -rf ../../output
    ./compile_build_bundle.sh $QMAKE $CORES
fi
./create_packages.sh $QMAKE
cd ../installer
rm -rf $TARGET_DIR
tclsh assemble.tcl $TARGET_DIR --repo

rm -rf $TMP_DIR
$TARGET_DIR/InstallSQLiteStudio-*.app/Contents/MacOS/Install* TargetDir=$TMP_DIR

mv $TMP_DIR $TARGET_DIR/
cd $TARGET_DIR

VER=`grep -A 1 ShortVersionString SQLiteStudio.app/Contents/Info.plist | tail -n 1 | egrep -o '([0-9]{1,}\.){1,}[0-9]{1,}'`
ditto -c -k --keepParent SQLiteStudio.app sqlitestudio-$VER.zip
hdiutil create SQLiteStudio-$VER-tmp.dmg -ov -volname "SQLiteStudio 3.2.0" -fs HFS+ -srcfolder "SQLiteStudio.app"
hdiutil convert SQLiteStudio-$VER-tmp.dmg -format UDZO -o SQLiteStudio-$VER.dmg

rm -rf SQLiteStudio-$VER-tmp.dmg
rm -rf SQLiteStudio.app
rm -rf $TARGET_DIR/InstallSQLiteStudio-*.app
echo "Deleted installed app. System is clean."

cd $OLDDIR
