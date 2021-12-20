#!/bin/sh

path=trunk
if [ "$1" != "" ]; then
    path=branches/$1
fi

OLDDIR=`pwd`

TEMP=`mktemp -d`
cd $TEMP

#svn co svn://sqlitestudio.pl/sqlitestudio3/$path sqlitestudio
git clone https://github.com/pawelsalawa/sqlitestudio.git sqlitestudio

cd sqlitestudio
rm -rf .git .gitignore

VERSION_INT=`cat SQLiteStudio3/coreSQLiteStudio/sqlitestudio.cpp | grep static | grep sqlitestudioVersion | sed 's/\;//'`
VERSION=`echo $VERSION_INT | awk '{print int($6/10000) "." int($6/100%100) "." int($6%100)}'`

tar cf ../sqlitestudio-$VERSION.tar SQLiteStudio3 Plugins
gzip -9 ../sqlitestudio-$VERSION.tar

zip -r ../sqlitestudio-$VERSION.zip SQLiteStudio3 Plugins

cd "$OLDDIR"

mv $TEMP/sqlitestudio-$VERSION.zip ../output
mv $TEMP/sqlitestudio-$VERSION.tar.gz ../output

cd ../output

rm -rf $TEMP

echo "Source packages stored in `pwd`"

cd "$OLDDIR"
