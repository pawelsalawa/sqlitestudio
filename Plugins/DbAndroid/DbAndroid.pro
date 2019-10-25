#-------------------------------------------------
#
# Project created by QtCreator 2015-01-04T19:37:23
#
#-------------------------------------------------

QT += widgets network

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = DbAndroid
TEMPLATE = lib

DEFINES += DBANDROID_LIBRARY

SOURCES += dbandroid.cpp \
    dbandroidinstance.cpp \
    sqlqueryandroid.cpp \
    dbandroidurl.cpp \
    dbandroidpathdialog.cpp \
    adbmanager.cpp \
    sqlresultrowandroid.cpp \
    dbandroidjsonconnection.cpp \
    dbandroidshellconnection.cpp \
    dbandroidconnection.cpp \
    dbandroidconnectionfactory.cpp

HEADERS += dbandroid.h\
        dbandroid_global.h \
    dbandroidinstance.h \
    sqlqueryandroid.h \
    dbandroidurl.h \
    dbandroidpathdialog.h \
    adbmanager.h \
    dbandroidconnection.h \
    dbandroidmode.h \
    sqlresultrowandroid.h \
    dbandroidjsonconnection.h \
    dbandroidshellconnection.h \
    dbandroidconnectionfactory.h

win32: {
    LIBS += -lcoreSQLiteStudio -lguiSQLiteStudio
}

DISTFILES += \
    dbandroid.json

FORMS += \
    dbandroidpathdialog.ui

RESOURCES += \
    dbandroid.qrc

TRANSLATIONS += translations/DbAndroid.ts \
		translations/DbAndroid_ro_RO.ts \
		translations/DbAndroid_de_DE.ts \
		\
		translations/DbAndroid_it_IT.ts\
		translations/DbAndroid_zh_CN.ts\
		translations/DbAndroid_sk_SK.ts\
		translations/DbAndroid_ru_RU.ts\
		translations/DbAndroid_pt_BR.ts\
		translations/DbAndroid_fr_FR.ts\
		translations/DbAndroid_es_ES.ts\
		translations/DbAndroid_pl_PL.ts















