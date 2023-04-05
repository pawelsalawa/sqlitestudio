#-------------------------------------------------
#
# Project created by QtCreator 2014-07-19T12:58:14
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

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
    message("Looking for $$TCL_CONFIG")
    !exists($$TCL_CONFIG) {
        TCL_CONFIG = $$TCL_CONFIG_DIR/../tclConfig.sh
	message("Looking for $$TCL_CONFIG")
    }
    !exists($$TCL_CONFIG) {
	# Debian, FreeBSD, Ubuntu Bionic case
        TCL_CONFIG = $$system(echo "puts [::tcl::pkgconfig get libdir,runtime]" | tclsh)/tcl$$TCL_VERSION/tclConfig.sh
    }
    message("Looking for $$TCL_CONFIG")
    !exists($$TCL_CONFIG) {
	error("Could not find tclConfig.sh file. You can define its absolute path by qmake parameter: TCL_CONFIG=/path/to/tclConfig.sh")
    }
    message("Using tclconfig: $$TCL_CONFIG")

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

macx: {
    # Find tclsh
    TCLSH = $$system(echo "puts 1" | tclsh)
    !contains(TCLSH, 1): {
        error("Could not find tclsh executable. ScriptingTcl plugin requires it to find out all Tcl libraries and headers. Make tclsh available in PATH.")
    }
    TCLSH = $$system(which tclsh)

    # Find its version
    TCL_VERSION = $$system(echo "puts [info tclversion]" | tclsh)
    message("Found tclsh: $$TCLSH (version: $$TCL_VERSION)")

    # Find tclConfig.sh
    TCL_CONFIG = $$system(echo "puts [::tcl::pkgconfig get libdir,runtime]" | tclsh)/tclConfig.sh

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
    # Under Windows we don't do the research. We just assume we have everything in the lib/ and include/
    # directories, which contain all other dependencies for SQLiteStudio. Get them from any Tcl installation you want.
    # Lib files required for compilation of this plugin:
    # - tcl86.lib
    # - tcl86.dll
    # Include files required for compilation:
    # - tcl.h
    # - tclDecls.h
    # - tclPlatDecls.h
    # Lib files required for the runtime in applications directory:
    # - tcl86.dll
    # The "86" part may vary, depending on Tcl version you're linking with.
    LIBS += -ltcl86
}

RESOURCES += \
    scriptingtcl.qrc
