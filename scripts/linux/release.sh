#!/bin/sh

set -e

TARGET_DIR=/tmp/sqls-installer
QMAKE=$(which qmake)
CORES=2
TMP_DIR=~/tmp/SQLiteStudio

if [ ! -f "$QMAKE" ]; then
    echo "qmake at $QMAKE does not exist."
    exit 1
fi

OLDDIR=$(pwd)

VER=$(grep 'static const int sqlitestudioVersion' ../../SQLiteStudio3/coreSQLiteStudio/sqlitestudio.cpp | grep -E -o '[\ 0-9]{2}' | bc | paste -s -d'.')

rm -rf ../../output
./compile.sh "$QMAKE" $CORES
./create_portable.sh "$QMAKE"
cd ../installer
rm -rf $TARGET_DIR
tclsh assemble.tcl $TARGET_DIR --repo

rm -rf $TMP_DIR
"$TARGET_DIR"/InstallSQLiteStudio-* TargetDir=$TMP_DIR

mv $TMP_DIR $TARGET_DIR/
cd $TARGET_DIR

tar cf "sqlitestudio-$VER.tar" SQLiteStudio
xz -z "sqlitestudio-$VER.tar"
rm -rf SQLiteStudio
echo "Deleted installed app. System is clean."

cd "$OLDDIR" || exit 1
