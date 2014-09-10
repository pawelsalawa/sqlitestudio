#!/bin/sh

if [ "$#" -ne 2 ]; then
  echo "$0 <sqlitestudio build output directory> <qmake path>"
  exit 1
fi

which chrpath >/dev/null
if [ "$?" -ne 0 ]; then
  echo "chrpath program missing!"
  exit 1
fi

qt_paths_bin="${2/qmake/qtpaths}"
$qt_paths_bin -v >/dev/null 2>&1
if [ "$?" -ne 0 ]; then
  echo "qtpaths program missing!"
  exit 1
fi

cd $1

required_modules="libQt5Core.so libQt5Concurrent.so libQt5Gui.so libQt5Network.so libQt5PrintSupport.so libQt5Script.so libQt5Widgets.so libQt5Xml.so libQt5Svg.so"
required_plugins="platforms/libqxcb.so imageformats/libqgif.so imageformats/libqicns.so imageformats/libqico.so imageformats/libqjpeg.so imageformats/libqmng.so \
  imageformats/libqsvg.so imageformats/libqtga.so imageformats/libqtiff.so iconengines/libqsvgicon.so printsupport/libcupsprintersupport.so platformthemes/libqgtk2.so"

qt_lib_dir=`ldd SQLiteStudio/sqlitestudio | grep libQt5Core | awk '{print $3;}'`
qt_lib_dir=`dirname $qt_lib_dir`
qt_plugins_dir=`$qt_paths_bin --plugin-dir`

# Create portable dir to store distribution in
rm -rf portable
mkdir portable
cd portable
portable=`pwd`

# Copy all output from compilation here
cp -R $1/SQLiteStudio .

# Make lib directory to move all *.so files (sqlitestudio files and Qt files and dependencies)
cd SQLiteStudio
mkdir lib
cd lib
mv ../*.so .
mv ../*.so.* .

# Copy Qt
cd $portable/SQLiteStudio/lib
for module in $required_modules; do
  if [ ! -f $qt_lib_dir/$module ]; then
    echo "Required Qt module doesn't exist: $qt_lib_dir/$module"
    exit 1
  fi
  cp -P $qt_lib_dir/$module* .
  
  for dep_lib in `ldd $qt_lib_dir/$module | grep $qt_lib_dir | awk '{print $3;}'`; do
    cp -Pu $dep_lib* .
  done
done

for lib in `ls *.so`; do
  
done

# Now copy Qt plugins
cd $portable/SQLiteStudio
for plugin in $required_plugins; do
  if [ ! -f $qt_plugins_dir/$plugin ]; then
    echo "Required Qt plugin doesn't exist: $qt_plugins_dir/$plugin"
    exit 1
  fi
  parts=(${plugin/\// })
  mkdir ${parts[0]} 2>/dev/null
  cp -P $qt_plugins_dir/$plugin ${parts[0]}
done
