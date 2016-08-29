#!/bin/sh

printUsage() {
  echo "$0 <sqlitestudio build output directory> <qmake path> [tgz|dist|dist_full]"
}

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  printUsage
  exit 1
fi

if [ "$#" -eq 3 ] && [ "$3" != "tgz" ] && [ "$3" != "dist" ] && [ "$3" != "dist_plugins" ] && [ "$3" != "dist_full" ]; then
  printUsage
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

required_modules="libQt5Core.so libQt5Concurrent.so libQt5DBus.so libQt5Gui.so libQt5Network.so libQt5PrintSupport.so libQt5Script.so libQt5Widgets.so libQt5Xml.so \
  libQt5Svg.so"
required_plugins="platforms/libqxcb.so imageformats/libqgif.so imageformats/libqicns.so imageformats/libqico.so imageformats/libqjpeg.so \
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

# Copy SQLite libs
cd $portable/SQLiteStudio
sqlite3_lib=`ldd $1/SQLiteStudio/lib/libcoreSQLiteStudio.so | grep libsqlite | awk '{print $3;}'`
sqlite2_lib=`ldd plugins/libDbSqlite2.so | grep libsqlite | awk '{print $3;}'`
cp $sqlite3_lib lib
cp $sqlite2_lib lib
strip lib/*libsqlite*

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
  chrpath -r \$ORIGIN/lib $lib >/dev/null
done

# Now copy Qt plugins
cd $portable/SQLiteStudio
qt_plugin_dirs=()
for plugin in $required_plugins; do
  if [ ! -f $qt_plugins_dir/$plugin ]; then
    echo "Required Qt plugin doesn't exist: $qt_plugins_dir/$plugin"
    exit 1
  fi
  parts=(${plugin/\// })
  mkdir ${parts[0]} 2>/dev/null
  cp -P $qt_plugins_dir/$plugin ${parts[0]}
  
  # Update rpath in Qt plugins
  cd ${parts[0]}
  for lib in `ls *.so`; do
    chrpath -r \$ORIGIN/../lib $lib >/dev/null
  done
  cd ..
done

cd $portable/SQLiteStudio
chrpath -r \$ORIGIN/lib sqlitestudio >/dev/null
chrpath -r \$ORIGIN/lib sqlitestudiocli >/dev/null

cd $portable
VERSION=`SQLiteStudio/sqlitestudiocli -v | awk '{print $2}'`

if [ "$3" == "tgz" ]; then
  tar cf sqlitestudio-$VERSION.tar SQLiteStudio
  xz -z sqlitestudio-$VERSION.tar
elif [ "$3" == "dist" ] || [ "$3" == "dist_plugins" ] || [ "$3" == "dist_full" ]; then
  if [ "$3" == "dist" ] || [ "$3" == "dist_full" ]; then
    # Complete
    echo "Building complete package: sqlitestudio-$VERSION.tar.xz"
    tar cf sqlitestudio-$VERSION.tar SQLiteStudio
    xz -z sqlitestudio-$VERSION.tar
  
    # App
    echo "Building incremental update package: sqlitestudio-$VERSION.tar.gz"
    cp -R SQLiteStudio app
    cd app
    if [ "$3" == "dist" ]; then
        rm -rf plugins
        rm -f lib/libQ*
        rm -rf iconengines
        rm -rf imageformats
        rm -rf platforms
        rm -rf platformthemes
        rm -rf printsupport
        find . -type l -exec rm -f {} \;
    fi
    rm -f lib/libicu*
    rm -f lib/libsqlite.so.0 ;# this is for SQLite 2
    tar cf sqlitestudio-$VERSION.tar *
    gzip -9 sqlitestudio-$VERSION.tar
    mv sqlitestudio-$VERSION.tar.gz ..
    cd ..
    rm -rf app
  fi

  # Plugins
  mkdir plugins
  SQLiteStudio/sqlitestudio --list-plugins | while read line
  do
    PLUGIN=`echo $line | awk '{print $1}'`
    PLUGIN_VER=`echo $line | awk '{print $2}'`
    if [ -f SQLiteStudio/plugins/lib$PLUGIN.so ]; then
      echo "Building plugin package: $PLUGIN-$PLUGIN_VER.tar.gz"
      cp SQLiteStudio/plugins/lib$PLUGIN.so plugins/
      tar cf $PLUGIN\-$PLUGIN_VER.tar plugins
      gzip -9 $PLUGIN\-$PLUGIN_VER.tar
    fi
    rm -f plugins/*
  done
  rm -rf plugins
  
  echo "Done."
fi
