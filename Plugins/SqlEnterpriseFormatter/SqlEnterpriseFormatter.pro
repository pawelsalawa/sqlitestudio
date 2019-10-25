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
    formatstatement.cpp \
    formatselect.cpp \
    formatexpr.cpp \
    formatlimit.cpp \
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
    formatcopy.cpp \
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
    formatcopy.h \
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


TRANSLATIONS += translations/SqlEnterpriseFormatter_ro_RO.ts \
		translations/SqlEnterpriseFormatter_de_DE.ts \
		translations/SqlEnterpriseFormatter_it_IT.ts \
		translations/SqlEnterpriseFormatter_zh_CN.ts \
		translations/SqlEnterpriseFormatter_sk_SK.ts \
		translations/SqlEnterpriseFormatter_ru_RU.ts \
		translations/SqlEnterpriseFormatter_pt_BR.ts \
		translations/SqlEnterpriseFormatter_fr_FR.ts \
		translations/SqlEnterpriseFormatter_es_ES.ts \
		translations/SqlEnterpriseFormatter_pl_PL.ts


























