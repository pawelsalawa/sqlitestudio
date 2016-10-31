#!/bin/sh
realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

cdir=`pwd`
absolute_path=`realpath $0`
this_dir=`dirname $absolute_path`
parent_dir=`dirname $this_dir`
parent_dir=`dirname $parent_dir`
cd $parent_dir/output/build
make pkg

cd ..
mv SQLiteStudio/*.zip SQLiteStudio/*.dmg .
cd $cdir
