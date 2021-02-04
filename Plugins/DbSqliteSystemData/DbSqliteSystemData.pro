#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T10:00:38
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

DEFINES += DBSQLITESYSTEMDATA_LIBRARY

TARGET = DbSqliteSystemData
TEMPLATE = lib

SOURCES += dbsqlitesystemdata.cpp \
    dbsqlitesystemdatainstance.cpp \
    systemdata_crypt.c \
    systemdata_extension-functions.c \
    systemdata_fts5.c \
    systemdata_interop.c \
    systemdata_json1.c \
    systemdata_percentile.c \
    systemdata_regexp.c \
    systemdata_sha1.c \
    systemdata_sqlite3.c \
    systemdata_totype.c \
    systemdata_vtshim.c

HEADERS += dbsqlitesystemdata.h \
    dbsqlitesystemdatainstance.h \
    dbsqlitesystemdata_global.h \
    systemdata_interop.h \
    systemdata_sqlite3.h \
    systemdata_sqlite3ext.h

DISTFILES += DbSqliteSystemData.json

INCLUDEPATH += $${PWD}/../deps/include/$${PLATFORM}/
DEPENDPATH += $${PWD}/../deps/include/$${PLATFORM}/
LIBS += -lcoreSQLiteStudio

DEFINES += SQLITE_OS_WIN=1 SQLITE_HAS_CODEC SQLITE_ALLOW_XTHREAD_CONNECT=1 SQLITE_THREADSAFE=1 SQLITE_TEMP_STORE=2 \
    SQLITE_CORE USE_DYNAMIC_SQLITE3_LOAD=0 HAVE_ACOSH=1 HAVE_ASINH=1 HAVE_ATANH=1 HAVE_ISBLANK=1 \
    INTEROP_CODEC=1 INTEROP_EXTENSION_FUNCTIONS=1 SQLITE_ENABLE_LOAD_EXTENSION=1 \
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
    SQLITE_ENABLE_RTREE=1

QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function -Wno-unused-but-set-variable -Wno-parentheses

OTHER_FILES += \
    dbsqlitesystemdata.json


