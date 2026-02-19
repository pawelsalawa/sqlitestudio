cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(version)

sqlitestudio_find_version()
project(SQLiteStudio3 VERSION "${SQLITESTUDIO_VERSION_STRING}")

if(WIN32 OR APPLE)
    set(WITH_PORTABLE_DEFAULT ON)
endif()

option(WITH_CLI "Build CLI application" ON)
option(WITH_GUI "Build GUI application" ON)

option(WITH_PORTABLE "Create portable bundle" ${WITH_PORTABLE_DEFAULT})
option(WITH_UPDATER "Enable update manager" ${WITH_PORTABLE})

if(NOT WITH_CLI AND NOT WITH_GUI)
    message(FATAL_ERROR "At least one of WITH_CLI or WITH_GUI must be enabled")
endif()

include(CTest)
include(common)

add_subdirectory(coreSQLiteStudio)
if(WITH_CLI)
    add_subdirectory(sqlitestudiocli)
endif()
if(WITH_GUI)
    add_subdirectory(guiSQLiteStudio)
    add_subdirectory(sqlitestudio)
endif()

if(BUILD_TESTING)
    add_subdirectory(Tests)
endif()
