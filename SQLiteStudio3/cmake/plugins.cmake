include(common)

# hack ui_*.h
find_package(Qt6 REQUIRED COMPONENTS Widgets)
file(GLOB_RECURSE guiSQLiteStudio_FORMS "${CMAKE_CURRENT_LIST_DIR}/../guiSQLiteStudio/*.ui")
qt_wrap_ui(guiSQLiteStudio_UI ${guiSQLiteStudio_FORMS})
add_custom_target(generate_ui DEPENDS ${guiSQLiteStudio_UI})

function(sqlitestudio_set_plugin_properties target)
    sqlitestudio_set_common_properties(${target})
    sqlitestudio_set_translations(${target})

    add_dependencies(${target} generate_ui)

    target_include_directories(${target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        "${CMAKE_BINARY_DIR}"
        "${DIR_OF_COMMON_CMAKE}/../coreSQLiteStudio"
        "${DIR_OF_COMMON_CMAKE}/../guiSQLiteStudio"
    )

	if(WIN32)
		target_link_directories(${target} PRIVATE "${CMAKE_INSTALL_PREFIX}")
		target_link_libraries(${target} PRIVATE coreSQLiteStudio)
		if(Qt6Gui_FOUND)
			target_link_libraries(${target} PRIVATE guiSQLiteStudio)
		endif()
	endif()

    install(
        TARGETS ${target}
        LIBRARY DESTINATION "${SQLITESTUDIO_INSTALL_PLUGINDIR}" # macOS/Linux (.dylib/.so)
		RUNTIME DESTINATION "${SQLITESTUDIO_INSTALL_PLUGINDIR}" # Windows (.dll)
    )
endfunction()


function(sqlitestudio_set_style_properties target)
    sqlitestudio_set_common_properties(${target})

    target_include_directories(${target} PRIVATE
        "${DIR_OF_COMMON_CMAKE}/../coreSQLiteStudio"
        "${DIR_OF_COMMON_CMAKE}/../guiSQLiteStudio"
    )

    install(
        TARGETS ${target}
        LIBRARY DESTINATION "${SQLITESTUDIO_INSTALL_STYLEDIR}" # macOS/Linux (.dylib/.so)
		RUNTIME DESTINATION "${SQLITESTUDIO_INSTALL_STYLEDIR}" # Windows (.dll)
    )
endfunction()
