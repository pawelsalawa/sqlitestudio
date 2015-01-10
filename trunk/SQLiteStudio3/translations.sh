#!/bin/sh

function printUsage()
{
    echo "$0 (update|release)"
}

if [ "$#" -lt 1 ]; then
    printUsage
    exit 1
fi

function doUpdate()
{
    lupdate SQLiteStudio3.pro
    find ../Plugins -name "*.pro" -exec lupdate {} \;
}

function doRelease()
{
    find ../SQLiteStudio3 -name "*.pro" -exec lrelease {} \;
    find ../Plugins -name "*.pro" -exec lrelease {} \;
}

case "$1" in
    "update")
	doUpdate
    ;;
    "release")
	doRelease
    ;;
    *)
	printUsage
	exit 1
esac
