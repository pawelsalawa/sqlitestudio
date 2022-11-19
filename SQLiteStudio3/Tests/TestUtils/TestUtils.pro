#-------------------------------------------------
#
# Project created by QtCreator 2013-03-29T23:25:17
#
#-------------------------------------------------

include(test_common.pri)

QT       -= gui

TARGET = TestUtils
TEMPLATE = lib

DEFINES += TESTUTILS_LIBRARY

SOURCES += \
    dbsqlite3mock.cpp \
    functionmanagermock.cpp \
    pluginmanagermock.cpp \
    configmock.cpp \
    mocks.cpp \
    dbattachermock.cpp \
    dbmanagermock.cpp \
    collationmanagermock.cpp \
    extensionmanagermock.cpp

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
    collationmanagermock.h \
    extensionmanagermock.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = $$PREFIX/lib
    }
    INSTALLS += target
}

LIBS += -lcoreSQLiteStudio
LIBS -= -lTestUtils

OTHER_FILES += \
    test_common.pri
