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
    dbsqlitecipherinstance.cpp \
    sqlcipher.c

HEADERS += dbsqlitecipher.h \
    dbsqlitecipher_global.h \
    dbsqlitecipherinstance.h \
    sqlcipher.h

!macx: {
    LIBS += -L$${PWD}/../deps/lib/$${PLATFORM}/
}
win32: {
    INCLUDEPATH += $${PWD}/../deps/include/$${PLATFORM}/
    DEPENDPATH += $${PWD}/../deps/include/$${PLATFORM}/
    LIBS += -leay32 -lcoreSQLiteStudio
}

!win32: {
    LIBS += -lcrypto
}

unix: {
    DEFINES += SQLITE_OS_UNIX=1
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

TRANSLATIONS += \
		DbSqliteCipher_it.ts\
		DbSqliteCipher_zh_CN.ts\
		DbSqliteCipher_sk.ts\
		DbSqliteCipher_de.ts\
		DbSqliteCipher_ru.ts\
		DbSqliteCipher_pt_BR.ts\
		DbSqliteCipher_fr.ts\
		DbSqliteCipher_es.ts\
		DbSqliteCipher_pl.ts

