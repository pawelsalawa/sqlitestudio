#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T12:07:32
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = DbSqliteCipher
TEMPLATE = lib

DEFINES += DBSQLITECIPHER_LIBRARY

SOURCES += dbsqlitecipher.cpp \
    dbsqlitecipherinstance.cpp

!unix|isEmpty(SQLCIPHER_LIB): {
    SOURCES += sqlcipher.c
}

HEADERS += dbsqlitecipher.h \
    dbsqlitecipher_global.h \
    dbsqlitecipherinstance.h
    sqlcipher.h

macx: {
    exists( /opt/local/include/openssl-3/openssl/crypto.h ) {
        message( "Configuring OpenSSL from MacPorts" )
        INCLUDEPATH += /opt/local/include/openssl-3
        LIBS += -L/opt/local/lib/openssl-3
    } else {
        message( "Configuring OpenSSL from HomeBrew" )
        INCLUDEPATH += /usr/local/opt/openssl/include
        LIBS += -L/usr/local/opt/openssl/lib
    }
}
win32: {
    LIBS += -lcoreSQLiteStudio
    !exists( ${{PWD}}/../../../../lib/libeay32.dll ): {
        LIBS += -lcrypto
    }
    exists( ${{PWD}}/../../../../lib/libeay32.dll ): {
        INCLUDEPATH += $${PWD}/../../../../include
        LIBS += -leay32
    }
}

!win32:isEmpty(SQLCIPHER_LIB) {
    LIBS += -lcrypto
}

unix: {
    DEFINES += SQLITE_OS_UNIX=1
    !isEmpty(SQLCIPHER_LIB): {
	LIBS += $$SQLCIPHER_LIB
	DEFINES += SQLCIPHER_SYSTEM_LIB
    }
}
win32: {
    DEFINES += SQLITE_OS_WIN=1
}
DEFINES += SQLITE_HAS_CODEC SQLCIPHER_CRYPTO_OPENSSL BUILD_sqlite NDEBUG SQLITE_ALLOW_XTHREAD_CONNECT=1 SQLITE_THREADSAFE=1 SQLITE_TEMP_STORE=2 \
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


OTHER_FILES += \
    dbsqlitecipher.json \
    sqlcipher.txt \
    sqlcipher_compiling.txt

RESOURCES += \
    dbsqlitecipher.qrc

QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable

DISTFILES += \
    openssl_lic.txt
