find_package(Qt6 REQUIRED COMPONENTS Core)

set(PROJECT_HEADERS
    clicommandexecutor.h
    clicommandsyntax.h
    clicompleter.h
    cli_config.h
    cli.h
    climsghandler.h
    cliutils.h
    commands/clicommandadd.h
    commands/clicommandcd.h
    commands/clicommandclose.h
    commands/clicommanddblist.h
    commands/clicommanddesc.h
    commands/clicommanddir.h
    commands/clicommandexit.h
    commands/clicommandfactory.h
    commands/clicommand.h
    commands/clicommandhelp.h
    commands/clicommandhistory.h
    commands/clicommandmode.h
    commands/clicommandnullvalue.h
    commands/clicommandopen.h
    commands/clicommandpwd.h
    commands/clicommandremove.h
    commands/clicommandsql.h
    commands/clicommandtables.h
    commands/clicommandtree.h
    commands/clicommanduse.h
)

set(PROJECT_SOURCES
    clicommandexecutor.cpp
    clicommandsyntax.cpp
    clicompleter.cpp
    cli_config.cpp
    cli.cpp
    climsghandler.cpp
    cliutils.cpp
    commands/clicommandadd.cpp
    commands/clicommandcd.cpp
    commands/clicommandclose.cpp
    commands/clicommand.cpp
    commands/clicommanddblist.cpp
    commands/clicommanddesc.cpp
    commands/clicommanddir.cpp
    commands/clicommandexit.cpp
    commands/clicommandfactory.cpp
    commands/clicommandhelp.cpp
    commands/clicommandhistory.cpp
    commands/clicommandmode.cpp
    commands/clicommandnullvalue.cpp
    commands/clicommandopen.cpp
    commands/clicommandpwd.cpp
    commands/clicommandremove.cpp
    commands/clicommandsql.cpp
    commands/clicommandtables.cpp
    commands/clicommandtree.cpp
    commands/clicommanduse.cpp
    main.cpp
)
qt_add_resources(PROJECT_SOURCES
    sqlitestudiocli.qrc
)

add_executable(sqlitestudiocli ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_output_properties(sqlitestudiocli)
target_link_libraries(sqlitestudiocli PRIVATE
    Qt::Core
    coreSQLiteStudio
)

if(WIN32)
    target_link_libraries(sqlitestudiocli PRIVATE
        edit_static
    )
endif()

if(UNIX)
    target_link_libraries(sqlitestudiocli PRIVATE
        readline
    )
endif()

install(
    TARGETS sqlitestudiocli
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
