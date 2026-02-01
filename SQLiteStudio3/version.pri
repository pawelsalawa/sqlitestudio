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

# Calculate version components using Python
# Create a small Python script file to avoid shell escaping issues
VERSION_CALC_SCRIPT = $$OUT_PWD/calc_version.py
VERSION_CALC_CONTENT = "v = $$SQLITESTUDIO_VERSION_INT$$escape_expand(\\n)print(str(v//10000) + ' ' + str((v//100)%100) + ' ' + str(v%100))"
write_file($$VERSION_CALC_SCRIPT, VERSION_CALC_CONTENT)

# Run the script
win32 {
    # Windows
    VERSION_CALC = $$system(python $$VERSION_CALC_SCRIPT 2>NUL, lines)
    isEmpty(VERSION_CALC) {
        VERSION_CALC = $$system(python3 $$VERSION_CALC_SCRIPT 2>NUL, lines)
    }
} else {
    # Unix-like
    VERSION_CALC = $$system(python3 $$VERSION_CALC_SCRIPT 2>/dev/null, lines)
    isEmpty(VERSION_CALC) {
        VERSION_CALC = $$system(python $$VERSION_CALC_SCRIPT 2>/dev/null, lines)
    }
}

# Parse the results if we got them
!isEmpty(VERSION_CALC) {
    VERSION_PARTS = $$split(VERSION_CALC, " ")
    SQLITESTUDIO_VERSION_MAJOR = $$member(VERSION_PARTS, 0)
    SQLITESTUDIO_VERSION_MINOR = $$member(VERSION_PARTS, 1)
    SQLITESTUDIO_VERSION_PATCH = $$member(VERSION_PARTS, 2)
}

# If Python is not available, show error with helpful message
isEmpty(SQLITESTUDIO_VERSION_MAJOR) {
    error("Could not calculate version components. Python is required for building SQLiteStudio. Please ensure Python is installed and in your PATH.")
}

# Create version string
SQLITESTUDIO_VERSION_STRING = "$${SQLITESTUDIO_VERSION_MAJOR}.$${SQLITESTUDIO_VERSION_MINOR}.$${SQLITESTUDIO_VERSION_PATCH}"

message("SQLiteStudio version: $$SQLITESTUDIO_VERSION_STRING ($$SQLITESTUDIO_VERSION_INT)")

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
DEFINES += SQLITESTUDIO_VERSION_STRING=\\\"$$SQLITESTUDIO_VERSION_STRING\\\"

