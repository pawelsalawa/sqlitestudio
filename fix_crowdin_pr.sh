#!/bin/sh
echo "
git pull
git checkout l10n_master
git pull
git merge master -X theirs
git push
git checkout master
"
