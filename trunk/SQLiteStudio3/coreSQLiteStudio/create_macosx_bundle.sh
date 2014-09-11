#!/bin/sh

printUsage() {
  echo "$0 <sqlitestudio build output directory> <qmake path> [nodmg]"
}

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  printUsage
  exit 1
fi

if [ "$#" -eq 3 ] && [ "$3" != "nodmg" ]; then
  printUsage
  exit 1
fi

qt_deploy_bin="${2/qmake/macdeployqt}"
$qt_deploy_bin -v >/dev/null 2>&1
if [ "$?" -ne 0 ]; then
  echo "macdeployqt program missing!"
  exit 1
fi

cd $1/SQLiteStudio

rm -rf SQLiteStudio.app/Contents/Frameworks
rm -rf SQLiteStudio.app/Contents/PlugIns
rm -f SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
rm -f SQLiteStudio.app/Contents/Resources/qt.conf

mkdir SQLiteStudio.app/Contents/Frameworks

cp -RP plugins SQLiteStudio.app/Contents
mv SQLiteStudio.app/Contents/plugins SQLiteStudio.app/Contents/PlugIns

cp -RP lib*SQLiteStudio*.dylib SQLiteStudio.app/Contents/Frameworks

qtcore_path=`otool -L sqlitestudiocli | grep QtCore | awk '{print $1;}'`
new_qtcore_path="@loader_path/../Frameworks/QtCore.framework/Versions/5/QtCore"

cp -P sqlitestudiocli SQLiteStudio.app/Contents/MacOS
install_name_tool -change libcoreSQLiteStudio.1.dylib "@loader_path/../Frameworks/libcoreSQLiteStudio.1.dylib" SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
install_name_tool -change $qtcore_path $new_qtcore_path SQLiteStudio.app/Contents/MacOS/sqlitestudiocli

cp -RP ../../../lib/*.dylib SQLiteStudio.app/Contents/Frameworks

if [ "$3" == "dmg" ]; then
    $qt_deploy_bin SQLiteStudio.app -dmg
else
    $qt_deploy_bin SQLiteStudio.app
fi
