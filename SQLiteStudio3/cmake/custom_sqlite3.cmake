# Custom SQLite (dedicated for SQLiteStudio official builds)
set(CUSTOM_SQLITE3 "" CACHE PATH "Path to directory where you placed your custom SQLite3 files (both library and header)")
add_library(SQLite::Headers INTERFACE)
if(CUSTOM_SQLITE3)
	message("Using custom SQLite3 from: ${CUSTOM_SQLITE3}")
	set(_sqlite_include "${CUSTOM_SQLITE3}")
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

	if(NOT EXISTS "${_sqlite_include}/sqlite3.h")
		message(FATAL_ERROR "SQLite3 include dir not found: ${_sqlite_include}")
	endif()

	target_include_directories(SQLite::Headers INTERFACE
		"${_sqlite_include}"
	)
	add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
	set_target_properties(SQLite::SQLite3 PROPERTIES
		IMPORTED_LOCATION "${_sqlite_lib}"
		INTERFACE_INCLUDE_DIRECTORIES "${_sqlite_include}"
	)
endif()
