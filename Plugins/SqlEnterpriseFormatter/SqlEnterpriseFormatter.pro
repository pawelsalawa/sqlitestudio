#-------------------------------------------------
#
# Project created by QtCreator 2014-09-11T10:57:25
#
#-------------------------------------------------

QT       -= gui

include($$PWD/../../SQLiteStudio3/plugins.pri)

TARGET = SqlEnterpriseFormatter
TEMPLATE = lib

DEFINES += SQLENTERPRISEFORMATTER_LIBRARY

SOURCES += sqlenterpriseformatter.cpp \
    formatfilterover.cpp \
    formatstatement.cpp \
    formatselect.cpp \
    formatexpr.cpp \
    formatlimit.cpp \
    formatwindowdefinition.cpp \
    formatwith.cpp \
    formatraise.cpp \
    formatcreatetable.cpp \
    formatforeignkey.cpp \
    formatcolumntype.cpp \
    formatindexedcolumn.cpp \
    formatinsert.cpp \
    formatempty.cpp \
    formataltertable.cpp \
    formatanalyze.cpp \
    formatattach.cpp \
    formatbegintrans.cpp \
    formatcommittrans.cpp \
    formatcreateindex.cpp \
    formatcreatetrigger.cpp \
    formatdelete.cpp \
    formatupdate.cpp \
    formatcreateview.cpp \
    formatcreatevirtualtable.cpp \
    formatdetach.cpp \
    formatdropindex.cpp \
    formatdroptable.cpp \
    formatdroptrigger.cpp \
    formatdropview.cpp \
    formatpragma.cpp \
    formatreindex.cpp \
    formatrelease.cpp \
    formatrollback.cpp \
    formatsavepoint.cpp \
    formatvacuum.cpp \
    formatorderby.cpp \
    formatupsert.cpp

HEADERS += sqlenterpriseformatter.h\
    formatfilterover.h \
    formatwindowdefinition.h \
        sqlenterpriseformatter_global.h \
    formatstatement.h \
    formatselect.h \
    formatexpr.h \
    formatlimit.h \
    formatwith.h \
    formatraise.h \
    formatcreatetable.h \
    formatforeignkey.h \
    formatcolumntype.h \
    formatindexedcolumn.h \
    formatinsert.h \
    formatempty.h \
    formataltertable.h \
    formatanalyze.h \
    formatattach.h \
    formatbegintrans.h \
    formatcommittrans.h \
    formatcreateindex.h \
    formatcreatetrigger.h \
    formatdelete.h \
    formatupdate.h \
    formatcreateview.h \
    formatcreatevirtualtable.h \
    formatdetach.h \
    formatdropindex.h \
    formatdroptable.h \
    formatdroptrigger.h \
    formatdropview.h \
    formatpragma.h \
    formatreindex.h \
    formatrelease.h \
    formatrollback.h \
    formatsavepoint.h \
    formatvacuum.h \
    formatorderby.h \
    formatupsert.h

OTHER_FILES += \
    sqlenterpriseformatter.json

FORMS += \
    sqlenterpriseformatter.ui

RESOURCES += \
    sqlenterpriseformatter.qrc























