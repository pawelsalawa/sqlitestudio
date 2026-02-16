cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(version)

sqlitestudio_find_version()
project(SQLiteStudio3 VERSION "${SQLITESTUDIO_VERSION_STRING}")

option(WITH_PORTABLE "Create portable bundle" $<OR:$<PLATFORM_ID:Windows>,$<PLATFORM_ID:Darwin>>)
option(WITH_UPDATER "Enable update manager" WITH_PORTABLE)
option(WITH_TESTS "Build tests" OFF)

include(common)

add_subdirectory(coreSQLiteStudio)
add_subdirectory(guiSQLiteStudio)
add_subdirectory(sqlitestudio)
add_subdirectory(sqlitestudiocli)
if(WITH_TESTS)
    # enable `ctest` at build dir
    enable_testing()
    add_subdirectory(Tests)
endif()
