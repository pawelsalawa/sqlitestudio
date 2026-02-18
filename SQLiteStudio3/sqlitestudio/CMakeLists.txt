find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Widgets)

set(PROJECT_HEADERS
    singleapplication/singleapplication.h
    singleapplication/singleapplication_p.h
)

set(PROJECT_SOURCES
    ../guiSQLiteStudio/img/sqlitestudio.icns
    main.cpp
    singleapplication/singleapplication.cpp
)
set_source_files_properties(../guiSQLiteStudio/img/sqlitestudio.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)
qt_add_resources(PROJECT_SOURCES
    sqlitestudio.qrc
)

add_executable(sqlitestudio ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_output_properties(sqlitestudio)
set_target_properties(sqlitestudio PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
target_compile_definitions(sqlitestudio PRIVATE
    QAPPLICATION_CLASS=QApplication
)
target_link_libraries(sqlitestudio PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
    coreSQLiteStudio
    guiSQLiteStudio
)

if(WIN32)
    if(MSVC)
        target_link_libraries(sqlitestudio PRIVATE
            Advapi32.lib
            User32.lib
        )
    else()
        target_link_libraries(sqlitestudio PRIVATE
            Advapi32
            User32
        )
    endif()
endif()

install(
    TARGETS sqlitestudio
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if(UNIX AND NOT APPLE)
    file(GLOB ICONS "../guiSQLiteStudio/img/sqlitestudio_[0-9]*.png")
    if(WITH_PORTABLE)
        install(
            FILES
            sqlitestudio.desktop
            ../guiSQLiteStudio/img/sqlitestudio.svg
            ${ICONS}
            DESTINATION "${CMAKE_INSTALL_DATADIR}"
        )
    else()
        install(
            FILES sqlitestudio.desktop
            DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
        )
        install(
            FILES ../guiSQLiteStudio/img/sqlitestudio.svg
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
        )
        foreach(ICON ${ICONS})
            if(NOT (ICON MATCHES "_([0-9]+).png"))
                message(WARNING "Could not extract icon size from '${ICON}'")
            else()
                set(ICON_SIZE ${CMAKE_MATCH_1})
                install(
                    FILES "${ICON}"
                    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps"
                )
            endif()
        endforeach()
    endif()
endif()
