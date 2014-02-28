#-------------------------------------------------
#
# Project created by QtCreator 2013-02-28T23:00:28
#
#-------------------------------------------------

include($$PWD/../dirs.pri)

OBJECTS_DIR = $$OBJECTS_DIR/coreSQLiteStudio
MOC_DIR = $$MOC_DIR/coreSQLiteStudio
UI_DIR = $$UI_DIR/coreSQLiteStudio

QT       -= gui
QT       += sql

TARGET = coreSQLiteStudio
TEMPLATE = lib

win32 {
    INCLUDEPATH += ../../include
    LIBS += -lpsapi
}

DEFINES += CORESQLITESTUDIO_LIBRARY

QMAKE_CXXFLAGS += -std=c++11 -pedantic

SOURCES += sqlitestudio.cpp \
    returncode.cpp \
    config.cpp \
    nulldevice.cpp \
    parser/lexer_low_lev.cpp \
    utils.cpp \
    parser/keywords.cpp \
    utils_sql.cpp \
    parser/token.cpp \
    parser/lexer.cpp \
    parser/sqlite3_parse.cpp \
    parser/parsercontext.cpp \
    parser/parser.cpp \
    parser/sqlite2_parse.cpp \
    parser/ast/sqlitestatement.cpp \
    parser/ast/sqlitequery.cpp \
    parser/ast/sqlitealtertable.cpp \
    parser/ast/sqliteanalyze.cpp \
    parser/ast/sqlitebegintrans.cpp \
    parser/ast/sqlitecommittrans.cpp \
    parser/ast/sqlitecreateindex.cpp \
    parser/ast/sqlitecreatetable.cpp \
    parser/ast/sqlitecreatetrigger.cpp \
    parser/ast/sqlitecreateview.cpp \
    parser/ast/sqlitecreatevirtualtable.cpp \
    parser/ast/sqlitedelete.cpp \
    parser/ast/sqlitedetach.cpp \
    parser/ast/sqlitedroptable.cpp \
    parser/ast/sqlitedroptrigger.cpp \
    parser/ast/sqlitedropindex.cpp \
    parser/ast/sqlitedropview.cpp \
    parser/ast/sqliteinsert.cpp \
    parser/ast/sqlitepragma.cpp \
    parser/ast/sqlitereindex.cpp \
    parser/ast/sqlitesavepoint.cpp \
    parser/ast/sqliterelease.cpp \
    parser/ast/sqliterollback.cpp \
    parser/ast/sqliteselect.cpp \
    parser/ast/sqliteupdate.cpp \
    parser/ast/sqlitevacuum.cpp \
    parser/ast/sqlitecopy.cpp \
    parser/ast/sqliteemptyquery.cpp \
    parser/parser_helper_stubs.cpp \
    parser/ast/sqliteexpr.cpp \
    parser/ast/sqliteforeignkey.cpp \
    parser/ast/sqliteindexedcolumn.cpp \
    parser/ast/sqlitecolumntype.cpp \
    parser/ast/sqliteconflictalgo.cpp \
    parser/ast/sqlitesortorder.cpp \
    parser/ast/sqliteraise.cpp \
    parser/ast/sqliteorderby.cpp \
    parser/ast/sqlitelimit.cpp \
    parser/ast/sqliteattach.cpp \
    parser/parsererror.cpp \
    selectresolver.cpp \
    schemaresolver.cpp \
    parser/ast/sqlitequerytype.cpp \
    db/db.cpp \
    db/dbmanager.cpp \
    db/sqlresults.cpp \
    db/sqlresultsrow.cpp \
    db/asyncqueryrunner.cpp \
    db/dbremote.cpp \
    completionhelper.cpp \
    completioncomparer.cpp \
    db/dbqt.cpp \
    db/dbpluginqt.cpp \
    pluginmanager.cpp \
    db/sqlresultsqt.cpp \
    db/queryexecutor.cpp \
    qio.cpp \
    pluginsymbolresolver.cpp \
    db/sqlresultsrowqt.cpp \
    db/sqlerrorresults.cpp \
    db/queryexecutorsteps/queryexecutorstep.cpp \
    db/queryexecutorsteps/queryexecutorcountresults.cpp \
    db/queryexecutorsteps/queryexecutorparsequery.cpp \
    db/queryexecutorsteps/queryexecutorexecute.cpp \
    db/queryexecutorsteps/queryexecutorattaches.cpp \
    db/queryexecutorsteps/queryexecutoraddrowids.cpp \
    db/queryexecutorsteps/queryexecutorlimit.cpp \
    db/queryexecutorsteps/queryexecutorcolumns.cpp \
    db/queryexecutorsteps/queryexecutorcellsize.cpp \
    db/queryexecutorsteps/queryexecutororder.cpp \
    db/sqlerrorcodes.cpp \
    readwritelocker.cpp \
    db/queryexecutorsteps/queryexecutorwrapdistinctresults.cpp \
    csvformat.cpp \
    csvserializer.cpp \
    db/queryexecutorsteps/queryexecutordatasources.cpp \
    expectedtoken.cpp \
    sqlhistorymodel.cpp \
    db/queryexecutorsteps/queryexecutorexplainmode.cpp \
    notifymanager.cpp \
    parser/statementtokenbuilder.cpp \
    parser/ast/sqlitedeferrable.cpp \
    tablemodifier.cpp \
    db/chainexecutor.cpp \
    db/queryexecutorsteps/queryexecutorreplaceviews.cpp \
    cfginternals.cpp \
    sqlformatter.cpp \
    viewmodifier.cpp \
    log.cpp \
    plugintype.cpp \
    genericplugin.cpp \
    memoryusage.cpp \
    ddlhistorymodel.cpp \
    datatype.cpp \
    table.cpp \
    column.cpp \
    dbattacher.cpp \
    db/dbqt2.cpp \
    functionmanager.cpp \
    db/dbqt3.cpp

HEADERS += sqlitestudio.h\
        coreSQLiteStudio_global.h \
    returncode.h \
    config.h \
    nulldevice.h \
    parser/lexer_low_lev.h \
    utils.h \
    parser/keywords.h \
    parser/token.h \
    utils_sql.h \
    parser/lexer.h \
    parser/sqlite3_parse.h \
    parser/parsercontext.h \
    parser/parser.h \
    parser/sqlite2_parse.h \
    parser/ast/sqlitestatement.h \
    parser/ast/sqlitequery.h \
    parser/ast/sqlitealtertable.h \
    parser/ast/sqliteanalyze.h \
    parser/ast/sqlitebegintrans.h \
    parser/ast/sqlitecommittrans.h \
    parser/ast/sqlitecreateindex.h \
    parser/ast/sqlitecreatetable.h \
    parser/ast/sqlitecreatetrigger.h \
    parser/ast/sqlitecreateview.h \
    parser/ast/sqlitecreatevirtualtable.h \
    parser/ast/sqlitedelete.h \
    parser/ast/sqlitedetach.h \
    parser/ast/sqlitedroptable.h \
    parser/ast/sqlitedroptrigger.h \
    parser/ast/sqlitedropindex.h \
    parser/ast/sqlitedropview.h \
    parser/ast/sqliteinsert.h \
    parser/ast/sqlitepragma.h \
    parser/ast/sqlitereindex.h \
    parser/ast/sqlitesavepoint.h \
    parser/ast/sqliterelease.h \
    parser/ast/sqliterollback.h \
    parser/ast/sqliteselect.h \
    parser/ast/sqliteupdate.h \
    parser/ast/sqlitevacuum.h \
    parser/ast/sqlitecopy.h \
    parser/ast/sqlitequerytype.h \
    parser/ast/sqliteemptyquery.h \
    parser/parser_helper_stubs.h \
    parser/ast/sqliteconflictalgo.h \
    parser/ast/sqliteexpr.h \
    parser/ast/sqliteforeignkey.h \
    parser/ast/sqliteindexedcolumn.h \
    parser/ast/sqlitecolumntype.h \
    parser/ast/sqlitesortorder.h \
    parser/ast/sqlitedeferrable.h \
    parser/ast/sqliteraise.h \
    parser/ast/sqliteorderby.h \
    parser/ast/sqlitelimit.h \
    parser/ast/sqliteattach.h \
    parser/parsererror.h \
    objectpool.h \
    selectresolver.h \
    schemaresolver.h \
    dialect.h \
    db/db.h \
    db/dbmanager.h \
    db/sqlresults.h \
    db/sqlresultsrow.h \
    db/asyncqueryrunner.h \
    db/dbremote.h \
    completionhelper.h \
    expectedtoken.h \
    completioncomparer.h \
    db/dbqt.h \
    db/dbplugin.h \
    db/dbpluginqt.h \
    pluginmanager.h \
    db/sqlresultsqt.h \
    db/queryexecutor.h \
    qio.h \
    db/dbpluginoption.h \
    global.h \
    parser/ast/sqlitetablerelatedddl.h \
    pluginsymbolresolver.h \
    db/sqlresultsrowqt.h \
    db/sqlerrorresults.h \
    db/sqlerrorcodes.h \
    db/queryexecutorsteps/queryexecutorstep.h \
    db/queryexecutorsteps/queryexecutorcountresults.h \
    db/queryexecutorsteps/queryexecutorparsequery.h \
    db/queryexecutorsteps/queryexecutorexecute.h \
    db/queryexecutorsteps/queryexecutorattaches.h \
    db/queryexecutorsteps/queryexecutoraddrowids.h \
    db/queryexecutorsteps/queryexecutorlimit.h \
    db/queryexecutorsteps/queryexecutorcolumns.h \
    db/queryexecutorsteps/queryexecutorcellsize.h \
    unused.h \
    db/queryexecutorsteps/queryexecutororder.h \
    readwritelocker.h \
    db/queryexecutorsteps/queryexecutorwrapdistinctresults.h \
    csvformat.h \
    csvserializer.h \
    db/queryexecutorsteps/queryexecutordatasources.h \
    sqlhistorymodel.h \
    db/queryexecutorsteps/queryexecutorexplainmode.h \
    notifymanager.h \
    parser/statementtokenbuilder.h \
    tablemodifier.h \
    db/chainexecutor.h \
    db/queryexecutorsteps/queryexecutorreplaceviews.h \
    cfginternals.h \
    sqlformatterplugin.h \
    sqlformatter.h \
    viewmodifier.h \
    log.h \
    plugintype.h \
    plugin.h \
    genericplugin.h \
    memoryusage.h \
    ddlhistorymodel.h \
    datatype.h \
    generalpurposeplugin.h \
    table.h \
    column.h \
    bihash.h \
    strhash.h \
    dbattacher.h \
    bistrhash.h \
    sqlfunctionplugin.h \
    db/dbqt2.h \
    functionmanager.h \
    db/dbqt3.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    parser/lempar.c \
    parser/sqlite3_parse.y \
    parser/sqlite2_parse.y \
    parser/run_lemon.sh \
    TODO.txt
