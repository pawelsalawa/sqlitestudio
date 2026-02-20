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

# Custom SQLite (dedicated for SQLiteStudio official builds)
set(CUSTOM_SQLITE3 "" CACHE PATH "Path to directory where you placed your custom SQLite3 files (both library and header)")
if(CUSTOM_SQLITE3)
	message("Using custom SQLite3 from: ${CUSTOM_SQLITE3}")
	set(_sqlite_include "${CUSTOM_SQLITE3}/sqlite3.h")
	if(WIN32)
		set(_sqlite_lib "${CUSTOM_SQLITE3}/libsqlite3.dll.a")
	elseif(APPLE)
		set(_sqlite_lib "${CUSTOM_SQLITE3}/libsqlite3.0.dylib")
	else()
		set(_sqlite_lib "${CUSTOM_SQLITE3}/libsqlite3.so")
	endif()

	if(NOT EXISTS "${_sqlite_lib}")
		message(FATAL_ERROR "SQLite3 library not found: ${_sqlite_lib}")
	endif()

	if(NOT EXISTS "${_sqlite_include}")
		message(FATAL_ERROR "SQLite3 include dir not found: ${_sqlite_include}")
	endif()
	
	add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
	set_target_properties(SQLite::SQLite3 PROPERTIES
		IMPORTED_LOCATION "${_sqlite_lib}"
		INTERFACE_INCLUDE_DIRECTORIES "${_sqlite_include}"
	)
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
