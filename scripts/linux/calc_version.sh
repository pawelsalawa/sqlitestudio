#!/usr/bin/env bash
set -euo pipefail

FILE="$(dirname "$0")/../../SQLiteStudio3/coreSQLiteStudio/sqlitestudio.cpp"

if [[ ! -f "$FILE" ]]; then
    echo "ERROR: File not found:" >&2
    echo "  $FILE" >&2
    exit 1
fi

LINE=$(grep -E 'sqlitestudioVersion[[:space:]]*=[^;]*;' "$FILE" | head -n1)

if [[ -z "$LINE" ]]; then
    echo "ERROR: sqlitestudioVersion not found" >&2
    exit 2
fi

VERSION=$(sed -E 's/.*=[[:space:]]*([0-9]+)[[:space:]]*;.*/\1/' <<<"$LINE")

MAJOR=$(( VERSION / 10000 ))
MINOR=$(( (VERSION / 100) % 100 ))
PATCH=$(( VERSION % 100 ))

echo "${MAJOR}.${MINOR}.${PATCH} $VERSION"