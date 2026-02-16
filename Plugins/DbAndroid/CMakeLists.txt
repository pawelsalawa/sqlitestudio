find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Widgets)

set(PROJECT_HEADERS
    adbmanager.h
    dbandroid_global.h
    dbandroid.h
    dbandroidconnection.h
    dbandroidconnectionfactory.h
    dbandroidinstance.h
    dbandroidjsonconnection.h
    dbandroidmode.h
    dbandroidpathdialog.h
    dbandroidshellconnection.h
    dbandroidurl.h
    sqlqueryandroid.h
    sqlresultrowandroid.h
)

set(PROJECT_SOURCES
    adbmanager.cpp
    dbandroid.cpp
    dbandroidconnection.cpp
    dbandroidconnectionfactory.cpp
    dbandroidinstance.cpp
    dbandroidjsonconnection.cpp
    dbandroidpathdialog.cpp
    dbandroidshellconnection.cpp
    dbandroidurl.cpp
    sqlqueryandroid.cpp
    sqlresultrowandroid.cpp
)
qt_add_resources(PROJECT_SOURCES
    dbandroid.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    dbandroidpathdialog.ui
)

add_library(DbAndroid SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(DbAndroid)
target_compile_definitions(DbAndroid PRIVATE
    DBANDROID_LIBRARY
)
target_link_libraries(DbAndroid PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)
