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



