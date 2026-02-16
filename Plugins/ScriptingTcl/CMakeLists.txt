find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(TCL)
if(NOT TCL_FOUND)
    message(FATAL_ERROR "TCL not found")
endif()

set(PROJECT_HEADERS
    scriptingtcl_global.h
    scriptingtcl.h
)

set(PROJECT_SOURCES
    scriptingtcl.cpp
)
qt_add_resources(PROJECT_SOURCES
    scriptingtcl.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    scriptingtcl.ui
)

add_library(ScriptingTcl SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(ScriptingTcl)
target_compile_definitions(ScriptingTcl PRIVATE
    SCRIPTINGTCL_LIBRARY
)
target_include_directories(ScriptingTcl PRIVATE
    ${TCL_INCLUDE_PATH}
)
target_link_libraries(ScriptingTcl PRIVATE
    Qt::Core
    ${TCL_LIBRARY}
)
