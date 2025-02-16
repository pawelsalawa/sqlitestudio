#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T10:00:38
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

DEFINES += DBSQLITEWX_LIBRARY

TARGET = DbSqliteWx
TEMPLATE = lib

SOURCES += dbsqlitewx.cpp \
    dbsqlitewxinstance.cpp

HEADERS += dbsqlitewx.h \
    dbsqlitewx_global.h \
    dbsqlitewxinstance.h

DISTFILES += DbSqliteWx.json

isEmpty(WXSQLITE_LIB): {
    SOURCES += wxsqlite3.c
    HEADERS += wxsqlite3.h
}

macx: {
    # exists( /opt/local/include/openssl-3/openssl/crypto.h ) {
    #     message( "Configuring OpenSSL from MacPorts" )
    #     INCLUDEPATH += /opt/local/include/openssl-3
    #     LIBS += -L/opt/local/lib/openssl-3
    # } else {
    #     message( "Configuring OpenSSL from HomeBrew" )
    #     INCLUDEPATH += /usr/local/opt/openssl/include
    #     LIBS += -L/usr/local/opt/openssl/lib
    # }
    LIBS += -framework Security
}
!macx: {
    # LIBS += -L$${PWD}/../deps/lib/$${PLATFORM}/
}
win32: {
    # INCLUDEPATH += $${PWD}/../deps/include/$${PLATFORM}/
    # DEPENDPATH += $${PWD}/../deps/include/$${PLATFORM}/
    LIBS += -lcoreSQLiteStudio
}

unix: {
    DEFINES += SQLITE_OS_UNIX=1
    !isEmpty(WXSQLITE_LIB): {
        LIBS += $$WXSQLITE_LIB
        DEFINES += WXSQLITE_SYSTEM_LIB
    }
}
win32: {
    DEFINES += SQLITE_OS_WIN=1
    # This one is to be removed after SQLiteStudio 3.4 (as mingw is updated and it's probably fixed there - to be checked if the plugin compiles without it)
    QMAKE_CFLAGS += -fno-asynchronous-unwind-tables
}
DEFINES += SQLITE_HAS_CODEC SQLITE_ALLOW_XTHREAD_CONNECT=1 SQLITE_THREADSAFE=1 SQLITE_TEMP_STORE=2 CODEC_TYPE=CODEC_TYPE_AES256 \
    SQLITE_CORE USE_DYNAMIC_SQLITE3_LOAD=0 \
    SQLITE_ENABLE_UPDATE_DELETE_LIMIT=1 \
    SQLITE_ENABLE_DBSTAT_VTAB=1 \
    SQLITE_ENABLE_BYTECODE_VTAB=1 \
    SQLITE_ENABLE_COLUMN_METADATA=1 \
    SQLITE_ENABLE_EXPLAIN_COMMENTS=1 \
    SQLITE_ENABLE_FTS3=1 \
    SQLITE_ENABLE_FTS3_PARENTHESIS=1 \
    SQLITE_ENABLE_FTS4=1 \
    SQLITE_ENABLE_FTS5=1 \
    SQLITE_ENABLE_GEOPOLY=1 \
    SQLITE_ENABLE_JSON1=1 \
    SQLITE_ENABLE_RTREE=1 \
    SQLITE_ENABLE_MATH_FUNCTIONS=1

# We cannot reliably detect the target architecture, assume that host == target
contains(QMAKE_HOST.arch,x86|x86_64|amd64): {
    QMAKE_CFLAGS += -msse4.1 -msse4.2 -maes
}

QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function -Wno-unused-but-set-variable \
    -Wno-parentheses -Wno-unused-variable -Wno-unknown-pragmas

OTHER_FILES += \
    dbsqlitewx.json
