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

!macx: {
    LIBS += -L$${PWD}/../deps/lib/$${PLATFORM}/
}
win32: {
    INCLUDEPATH += $${PWD}/../deps/include/$${PLATFORM}/
    DEPENDPATH += $${PWD}/../deps/include/$${PLATFORM}/
    LIBS += -leay32 -lcoreSQLiteStudio
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
DEFINES += SQLITE_HAS_CODEC SQLCIPHER_CRYPTO_OPENSSL BUILD_sqlite NDEBUG SQLITE_ALLOW_XTHREAD_CONNECT=1 SQLITE_THREADSAFE=1 SQLITE_TEMP_STORE=2

OTHER_FILES += \
    dbsqlitecipher.json \
    sqlcipher.txt \
    sqlcipher_compiling.txt

RESOURCES += \
    dbsqlitecipher.qrc

QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function

DISTFILES += \
    openssl_lic.txt

TRANSLATIONS += translations/DbSqliteCipher.ts \
		translations/DbSqliteCipher_ro_RO.ts \
	translations/DbSqliteCipher_de_DE.ts \
	\
	translations/DbSqliteCipher_it_IT.ts\
	translations/DbSqliteCipher_zh_CN.ts\
	translations/DbSqliteCipher_sk_SK.ts\
	translations/DbSqliteCipher_ru_RU.ts\
	translations/DbSqliteCipher_pt_BR.ts\
	translations/DbSqliteCipher_fr_FR.ts\
	translations/DbSqliteCipher_es_ES.ts\
	translations/DbSqliteCipher_pl_PL.ts












