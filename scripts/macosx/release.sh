#!/bin/sh

TARGET_DIR=/tmp/sqls-installer
QMAKE=~/Qt/5.10.0/clang_64/bin/qmake
CORES=3

if [ ! -f $QMAKE ]; then
    echo "qmake at $QMAKE does not exist."
    exit 1
fi

OLDDIR=`pwd`

rm -rf ../../output
./compile_build_bundle.sh $QMAKE $CORES
./create_packages.sh $QMAKE
cd ../installer
rm -rf $TARGET_DIR
tclsh assemble.tcl $TARGET_DIR --repo

$TARGET_DIR/InstallSQLiteStudio-*.app/Contents/MacOS/Install*

mv -R /Applications/SQLiteStudio.app $TARGET_DIR/
cd $TARGET_DIR

VER=`grep -A 1 ShortVersionString SQLiteStudio.app/Contents/Info.plist | tail -n 1 | egrep -o '([0-9]{1,}\.){1,}[0-9]{1,}'`
ditto -c -k --keepParent SQLiteStudio.app sqlitestudio-$VER.zip
rm -rf SQLiteStudio.app
echo "Deleted installed app. System is clean."

cd $OLDDIR
