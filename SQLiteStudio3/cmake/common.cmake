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
        qt_add_translations(${target}
            TS_FILES ${PROJECT_TRANSLATIONS}
            RESOURCE_PREFIX /msg/translations
        )
    endif()
endfunction()


function(sqlitestudio_set_test_properties target)
    sqlitestudio_set_common_properties(${target})

    target_link_libraries(${target} TestUtils)
    add_test(
        NAME ${target}
        COMMAND $<TARGET_FILE:${target}>
    )
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
            PORTABLE_CONFIG
        )
    endif()

    if(WITH_PORTABLE)
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
