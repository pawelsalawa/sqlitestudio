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
    dbsqlitewxinstance.cpp \
    chacha20poly1305.c \
    fastpbkdf2.c \
    fileio.c \
    md5.c \
    rekeyvacuum.c \
    sha1.c \
    shathree.c \
    test_windirent.c

!unix|isEmpty(WXSQLITE_LIB) {
    SOURCES += carray.c \
	codec.c \
	codecext.c \
	csv.c \
	extensionfunctions.c \
	rijndael.c \
	sha2.c \
	sqlite3secure.c \
	userauth.c
}

HEADERS += dbsqlitewx.h \
    codec.h \
    rijndael.h \
    sha2.h \
    sqlite3ext.h \
    sqlite3userauth.h \
    dbsqlitewx_global.h \
    dbsqlitewxinstance.h \
    fastpbkdf2.h \
    sha1.h \
    sqlite3secure.h \
    test_windirent.h

DISTFILES += DbSqliteWx.json

max: {
    INCLUDEPATH += /usr/local/opt/openssl/include
    LIBS += -L/usr/local/opt/openssl/lib
}
!macx: {
    LIBS += -L$${PWD}/../deps/lib/$${PLATFORM}/
}
win32: {
    INCLUDEPATH += $${PWD}/../deps/include/$${PLATFORM}/
    DEPENDPATH += $${PWD}/../deps/include/$${PLATFORM}/
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
    SQLITE_ENABLE_RTREE=1

QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function -Wno-unused-but-set-variable -Wno-parentheses

OTHER_FILES += \
    dbsqlitewx.json
