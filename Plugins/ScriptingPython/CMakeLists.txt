option(WITH_DYNAMIC_PYTHON "Build with dynamic Python" WITH_PORTABLE)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Python REQUIRED COMPONENTS Development)

set(PROJECT_HEADERS
    scriptingpython_global.h
    scriptingpython.h
)

set(PROJECT_SOURCES
    scriptingpython.cpp
)
qt_add_resources(PROJECT_SOURCES
    scriptingpython.qrc
)
qt_wrap_ui(PROJECT_SOURCES
    scriptingpython.ui
)

if(WITH_DYNAMIC_PYTHON)
    list(APPEND PROJECT_HEADERS dynamicpythonapi.h)
    list(APPEND PROJECT_SOURCES dynamicpythonapi.cpp)
else()
    list(APPEND PROJECT_HEADERS staticpythonapi.h)
endif()

add_library(ScriptingPython SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(ScriptingPython)
target_compile_definitions(ScriptingPython PRIVATE
    SCRIPTINGPYTHON_LIBRARY
)
target_link_libraries(ScriptingPython PRIVATE
    Qt::Core
)

if(WITH_DYNAMIC_PYTHON)
    get_target_property(PYTHON_INCLUDE_DIRS Python::Python INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(ScriptingPython PRIVATE
        ${PYTHON_INCLUDE_DIRS}
    )
    target_compile_definitions(ScriptingPython PRIVATE
        PYTHON_DYNAMIC_BINDING
    )
else()
    target_link_libraries(ScriptingPython PRIVATE
        Python::Python
    )
endif()
