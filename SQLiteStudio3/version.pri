# SQLiteStudio version extraction from sqlitestudio.cpp
# This file parses the version number from the source code and makes it available
# to all qmake builds, including Windows QtCreator builds.
#
# Version format: XXYYZZ where XX = major, YY = minor, ZZ = patch
# Example: 40000 = 4.0.0, 30515 = 3.5.15

# Path to the sqlitestudio.cpp file containing the version
SQLITESTUDIO_CPP = $$PWD/coreSQLiteStudio/sqlitestudio.cpp

# Read the file content
SQLITESTUDIO_CPP_CONTENT = $$cat($$SQLITESTUDIO_CPP, lines)

# Find the line containing the version and extract the number
for(line, SQLITESTUDIO_CPP_CONTENT) {
    contains(line, ".*static const int sqlitestudioVersion.*") {
        # Extract the version number from the line
        # Line format: "static const int sqlitestudioVersion = 40000;"
        VERSION_LINE = $$line
        
        # Remove the variable name and everything before the equals sign
        VERSION_LINE = $$replace(VERSION_LINE, "static const int sqlitestudioVersion", "")
        VERSION_LINE = $$replace(VERSION_LINE, "=", "")
        
        # Remove the semicolon
        VERSION_LINE = $$replace(VERSION_LINE, ";", "")
        
        # Remove all whitespace
        VERSION_LINE = $$replace(VERSION_LINE, " ", "")
        
        SQLITESTUDIO_VERSION_INT = $$VERSION_LINE
    }
}

# Verify we found the version
isEmpty(SQLITESTUDIO_VERSION_INT) {
    error("Could not extract version from $$SQLITESTUDIO_CPP")
}

# Calculate version components
# For Windows, use batch file to avoid Python dependency
# For Unix-like systems, use shell script
win32 {
    # Windows: Create batch file for version calculation
    VERSION_CALC_SCRIPT = $$OUT_PWD/calc_version.bat
    
    # Batch file content for Windows integer division
    VERSION_CALC_CONTENT = "@echo off"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    VERSION_CALC_CONTENT += "set ver=$$SQLITESTUDIO_VERSION_INT"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    # Major = ver / 10000
    VERSION_CALC_CONTENT += "set /a major=ver/10000"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    # Minor = (ver / 100) % 100
    VERSION_CALC_CONTENT += "set /a minor=(ver/100)%%100"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    # Patch = ver % 100
    VERSION_CALC_CONTENT += "set /a patch=ver%%100"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    VERSION_CALC_CONTENT += "echo %major% %minor% %patch%"
    
    write_file($$VERSION_CALC_SCRIPT, VERSION_CALC_CONTENT)
    
    # Run the batch file
    VERSION_CALC = $$system($$VERSION_CALC_SCRIPT 2>NUL, lines)
} else {
    # Unix-like: Create shell script for version calculation
    VERSION_CALC_SCRIPT = $$OUT_PWD/calc_version.sh
    
    # Shell script content for integer division
    VERSION_CALC_CONTENT = "#!/bin/sh"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    VERSION_CALC_CONTENT += "ver=$$SQLITESTUDIO_VERSION_INT"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    # Major = ver / 10000
    VERSION_CALC_CONTENT += "major=\$$((ver/10000))"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    # Minor = (ver / 100) % 100
    VERSION_CALC_CONTENT += "minor=\$$((ver/100%100))"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    # Patch = ver % 100
    VERSION_CALC_CONTENT += "patch=\$$((ver%100))"
    VERSION_CALC_CONTENT += $$escape_expand(\\n)
    VERSION_CALC_CONTENT += "echo \$$major \$$minor \$$patch"
    
    write_file($$VERSION_CALC_SCRIPT, VERSION_CALC_CONTENT)
    
    # Run the shell script
    VERSION_CALC = $$system(sh $$VERSION_CALC_SCRIPT 2>/dev/null, lines)
}

# Parse the results if we got them
!isEmpty(VERSION_CALC) {
    VERSION_PARTS = $$split(VERSION_CALC, " ")
    SQLITESTUDIO_VERSION_MAJOR = $$member(VERSION_PARTS, 0)
    SQLITESTUDIO_VERSION_MINOR = $$member(VERSION_PARTS, 1)
    SQLITESTUDIO_VERSION_PATCH = $$member(VERSION_PARTS, 2)
}

# If script execution failed, show error with helpful message
isEmpty(SQLITESTUDIO_VERSION_MAJOR) {
    error("Could not calculate version components from $$SQLITESTUDIO_VERSION_INT. Please check that the build environment supports batch/shell scripts.")
}

# Create version string
SQLITESTUDIO_VERSION_STRING = "$${SQLITESTUDIO_VERSION_MAJOR}.$${SQLITESTUDIO_VERSION_MINOR}.$${SQLITESTUDIO_VERSION_PATCH}"

message("SQLiteStudio version: $$SQLITESTUDIO_VERSION_STRING ($$SQLITESTUDIO_VERSION_INT)")

# Set qmake VERSION variable for use in .rc files and other contexts
VERSION = $$SQLITESTUDIO_VERSION_STRING

# Make variables available to other .pro files
export(SQLITESTUDIO_VERSION_INT)
export(SQLITESTUDIO_VERSION_MAJOR)
export(SQLITESTUDIO_VERSION_MINOR)
export(SQLITESTUDIO_VERSION_PATCH)
export(SQLITESTUDIO_VERSION_STRING)

# Add as DEFINES so they're available in C++ code if needed
DEFINES += SQLITESTUDIO_VERSION_INT=$$SQLITESTUDIO_VERSION_INT
DEFINES += SQLITESTUDIO_VERSION_MAJOR=$$SQLITESTUDIO_VERSION_MAJOR
DEFINES += SQLITESTUDIO_VERSION_MINOR=$$SQLITESTUDIO_VERSION_MINOR
DEFINES += SQLITESTUDIO_VERSION_PATCH=$$SQLITESTUDIO_VERSION_PATCH
# Triple-backslash escaping produces a quoted string literal in C++:
# e.g., #define SQLITESTUDIO_VERSION_STRING "4.0.0"
DEFINES += SQLITESTUDIO_VERSION_STRING=\\\"$$SQLITESTUDIO_VERSION_STRING\\\"

