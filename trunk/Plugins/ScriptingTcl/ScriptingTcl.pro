#-------------------------------------------------
#
# Project created by QtCreator 2014-07-19T12:58:14
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT       -= gui

TARGET = ScriptingTcl
TEMPLATE = lib

DEFINES += SCRIPTINGTCL_LIBRARY

SOURCES += scriptingtcl.cpp

HEADERS += scriptingtcl.h\
        scriptingtcl_global.h

OTHER_FILES += \
    scriptingtcl.json

linux: {
    # Find tclsh
    TCLSH = $$system(echo "puts 1" | tclsh)
    !contains(TCLSH, 1): {
        error("Could not find tclsh executable. ScriptingTcl plugin requires it to find out all Tcl libraries and headers. Make tclsh available in PATH.")
    }
    TCLSH = $$system(which tclsh)

    # Find its version
    TCL_VERSION = $$system(echo "puts [info tclversion]" | tclsh)
    #message("Found tclsh: $$TCLSH (version: $$TCL_VERSION)")

    # Find tclConfig.sh
    TCL_CONFIG_DIR = $$system(echo "puts [info library]" | tclsh)
    TCL_CONFIG = $$TCL_CONFIG_DIR/tclConfig.sh

    # Define other libs required when linking with Tcl
    eval($$system(cat $$TCL_CONFIG | grep TCL_LIBS))
    eval(LIBS += $$TCL_LIBS)

    # Define headers dir
    eval($$system(cat $$TCL_CONFIG | grep TCL_INCLUDE_SPEC))
    INCLUDEPATH += $$replace(TCL_INCLUDE_SPEC, -I/, /)
    DEPENDPATH += $$replace(TCL_INCLUDE_SPEC, -I/, /)

    # Find static library
    eval($$system(cat $$TCL_CONFIG | grep TCL_STUB_LIB_PATH))
    STATIC_LIB = $$replace(TCL_STUB_LIB_PATH, tclstub, tcl)

    # If found static lib, we link statically
    exists($$STATIC_LIB) {
        #message("Static linking of libtcl: $$STATIC_LIB")
        LIBS += $$STATIC_LIB
    }

    # If not found, use dynamic linking flags
    !exists($$STATIC_LIB) {
        eval($$system(cat $$TCL_CONFIG | grep TCL_LIB_SPEC))
        #message("Dynamic linking of libtcl: $$TCL_LIB_SPEC")
        eval(LIBS += $$TCL_LIB_SPEC)
    }
}

linux: {
    # Find tclsh
    TCLSH = $$system(echo "puts 1" | tclsh)
    !contains(TCLSH, 1): {
        error("Could not find tclsh executable. ScriptingTcl plugin requires it to find out all Tcl libraries and headers. Make tclsh available in PATH.")
    }
    TCLSH = $$system(which tclsh)

    # Find its version
    TCL_VERSION = $$system(echo "puts [info tclversion]" | tclsh)
    #message("Found tclsh: $$TCLSH (version: $$TCL_VERSION)")

    # Find tclConfig.sh
    TCL_CONFIG_DIR = $$system(echo "puts [info library]" | tclsh)
    TCL_CONFIG = $$TCL_CONFIG_DIR/../../tclConfig.sh

    # Define other libs required when linking with Tcl
    eval($$system(cat $$TCL_CONFIG | grep TCL_LIBS))
    eval(LIBS += $$TCL_LIBS)

    # Define headers dir
    eval($$system(cat $$TCL_CONFIG | grep TCL_INCLUDE_SPEC))
    INCLUDEPATH += $$replace(TCL_INCLUDE_SPEC, -I/, /)
    DEPENDPATH += $$replace(TCL_INCLUDE_SPEC, -I/, /)

    # Find static library
    eval($$system(cat $$TCL_CONFIG | grep TCL_STUB_LIB_PATH))
    STATIC_LIB = $$replace(TCL_STUB_LIB_PATH, tclstub, tcl)

    # If found static lib, we link statically
    exists($$STATIC_LIB) {
        #message("Static linking of libtcl: $$STATIC_LIB")
        LIBS += $$STATIC_LIB
    }

    # If not found, use dynamic linking flags
    !exists($$STATIC_LIB) {
        eval($$system(cat $$TCL_CONFIG | grep TCL_LIB_SPEC))
        #message("Dynamic linking of libtcl: $$TCL_LIB_SPEC")
        eval(LIBS += $$TCL_LIB_SPEC)
    }
}

win32: {
    # Find tclsh
    TCLSH = $$system(echo "puts 1" | tclsh)
    !contains(TCLSH, 1): {
        error("Could not find tclsh executable. ScriptingTcl plugin requires it to find out all Tcl libraries and headers. Make tclsh available in PATH.")
    }
    TCLSH = $$system(where tclsh)

    # Find its version
    TCL_VERSION = $$system(echo puts [info tclversion] | tclsh)
    #message("Found tclsh: $$TCLSH (version: $$TCL_VERSION)")

    # Find tclConfig.sh
    TCL_CONFIG_DIR = $$system(echo puts [info library] | tclsh)
    TCL_CONFIG = $$TCL_CONFIG_DIR/tclConfig.sh
    TCL_CONFIG = $$replace(TCL_CONFIG, /, \\)

    # Define headers dir
    eval($$system(type $$TCL_CONFIG | findstr /c:TCL_INCLUDE_SPEC))
    tcl_includes = $$replace(TCL_INCLUDE_SPEC, -I, "")
    INCLUDEPATH += $$tcl_includes
    DEPENDPATH += $$tcl_includes

    # Find static library
    eval($$system(type $$TCL_CONFIG | findstr /c:TCL_LIB_SPEC))
    eval(LIBS += $$TCL_LIB_SPEC)
    #LIBS += -LC:/Tcl/bin -LC:/Tcl/lib -ltcl86
}

RESOURCES += \
    scriptingtcl.qrc
