#-------------------------------------------------
#
# Project created by QtCreator 2013-03-29T23:25:17
#
#-------------------------------------------------

include(test_common.pri)

QT       -= gui

TARGET = TestUtils
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11

DEFINES += TESTUTILS_LIBRARY

SOURCES += \
    dbsqlite3mock.cpp \
    functionmanagermock.cpp \
    pluginmanagermock.cpp \
    configmock.cpp \
    mocks.cpp \
    dbattachermock.cpp \
    dbmanagermock.cpp \
    collationmanagermock.cpp

HEADERS +=\
        testutils_global.h \
    dbsqlite3mock.h \
    hippomocks.h \
    functionmanagermock.h \
    pluginmanagermock.h \
    configmock.h \
    mocks.h \
    dbattachermock.h \
    dbmanagermock.h \
    collationmanagermock.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

LIBS += -lcoreSQLiteStudio -lsqlite3

OTHER_FILES += \
    test_common.pri
