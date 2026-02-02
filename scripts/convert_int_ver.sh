#!/bin/sh
# Convert SQLiteStudio integer version to semver string
# Usage: convert_int_ver.sh <version_int>
# Example: convert_int_ver.sh 40000 → 4.0.0
#          convert_int_ver.sh 30515 → 3.5.15
#
# Version format: XXYYZZ where XX = major, YY = minor, ZZ = patch

if [ -z "$1" ]; then
    echo "Usage: $0 <version_int>" >&2
    echo "Example: $0 40000" >&2
    exit 1
fi

ver=$1

# Calculate version components using shell arithmetic
major=$((ver / 10000))
minor=$((ver / 100 % 100))
patch=$((ver % 100))

# Output version string
echo "${major}.${minor}.${patch}"
