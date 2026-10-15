#!/usr/bin/env bash
set -u

# For versions:
# https://github.com/utelle/SQLite3MultipleCiphers/releases
MC_VER="2.6.1"
SQLITE_VER="3.54.0"
THE_URL="https://github.com/utelle/SQLite3MultipleCiphers/releases/download/v${MC_VER}/sqlite3mc-${MC_VER}-sqlite-${SQLITE_VER}-amalgamation.zip"

FILES=(
    sqlite3mc_amalgamation.c
    sqlite3mc_amalgamation.h
)

copy_file() {
    local file="$1"
    local out_file

    out_file="$(basename "$file")"
    echo "Copying $out_file"

    sed \
        -e 's/sqlite3/mc_sqlite3/g' \
        -e 's/mc_sqlite3mc_amalgamation\./sqlite3mc_amalgamation./g' \
        "$file" > "$out_file"
}

process() {
    rm -rf sqlite.zip sqlite

    echo "Downloading $THE_URL"
    curl -L --fail -o sqlite.zip "$THE_URL" || return 1

    echo "Decompressing to 'sqlite' directory."
    7z x -osqlite sqlite.zip || return 1

    local dir="sqlite"

    for f in "${FILES[@]}"; do
        copy_file "$dir/$f" || return 1
    done
}

if ! process; then
    echo "Script failed." >&2
fi

rm -rf sqlite.zip sqlite
