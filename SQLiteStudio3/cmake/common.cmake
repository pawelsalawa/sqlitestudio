include(dirs)

set(DIR_OF_COMMON_CMAKE ${CMAKE_CURRENT_LIST_DIR})


function(sqlitestudio_set_common_properties target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
        AUTOMOC ON
        # Do not set AUTOUIC, the generated headers may be included by other
        # targets so we need a predicable location. Use qt_wrap_ui() instead.
    )

    target_include_directories(${target} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        # merely for generated ui_*.h
        "${CMAKE_CURRENT_BINARY_DIR}"
    )

    if(APPLE)
        target_compile_options(${target} PRIVATE
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-overloaded-virtual
        )
        target_link_directories(${target} PRIVATE
            /usr/local/lib
        )
    endif()

    if(NOT UNIX)
        target_include_directories(${target} PRIVATE
            "${DIR_OF_COMMON_CMAKE}/../../../include"
        )
        target_link_directories(${target} PRIVATE
            "${DIR_OF_COMMON_CMAKE}/../../../lib"
        )
    endif()
endfunction()


function(sqlitestudio_set_translations target)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/translations/")
        find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

        file(GLOB PROJECT_TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
        if(NOT WITH_SHARED_RES)
            qt_add_translations(${target}
                TS_FILES ${PROJECT_TRANSLATIONS}
                RESOURCE_PREFIX /msg/translations
            )
        else()
            qt_add_translations(${target}
                TS_FILES ${PROJECT_TRANSLATIONS}
                QM_FILES_OUTPUT_VARIABLE qm_files
            )
            install(
                FILES ${qm_files}
                DESTINATION "${SQLITESTUDIO_INSTALL_DATADIR}/translations"
            )
        endif()
    endif()
endfunction()


function(sqlitestudio_set_test_properties target)
    sqlitestudio_set_common_properties(${target})

    target_link_libraries(${target} TestUtils)
    add_test(
        NAME ${target}
        COMMAND $<TARGET_FILE:${target}>
    )

    if(BUILD_TESTING)
        install(TARGETS ${target}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT tests
        )
    endif()
endfunction()


function(sqlitestudio_set_output_properties target)
    sqlitestudio_set_common_properties(${target})
    sqlitestudio_set_translations(${target})

    target_compile_definitions(${target} PRIVATE
        SQLITESTUDIO_VERSION_INT=${SQLITESTUDIO_VERSION_INT}
        SQLITESTUDIO_VERSION_MAJOR=${SQLITESTUDIO_VERSION_MAJOR}
        SQLITESTUDIO_VERSION_MINOR=${SQLITESTUDIO_VERSION_MINOR}
        SQLITESTUDIO_VERSION_PATCH=${SQLITESTUDIO_VERSION_PATCH}
        SQLITESTUDIO_VERSION_STRING="${SQLITESTUDIO_VERSION_STRING}"
    )

    if(WITH_UPDATER)
        target_compile_definitions(${target} PUBLIC
            HAS_UPDATEMANAGER
        )
    endif()

    if(WITH_PORTABLE)
        target_compile_definitions(${target} PUBLIC
            PORTABLE_CONFIG
        )
        if(APPLE)
            set_target_properties(${target} PROPERTIES
                INSTALL_RPATH "@loader_path"
            )
        elseif(UNIX)
            set_target_properties(${target} PROPERTIES
                INSTALL_RPATH "$ORIGIN:$ORIGIN/lib"
            )
        endif()
    endif()
endfunction()
