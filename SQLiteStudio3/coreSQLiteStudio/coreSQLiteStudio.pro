#-------------------------------------------------
#
# Project created by QtCreator 2013-02-28T23:00:28
#
#-------------------------------------------------

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/coreSQLiteStudio
MOC_DIR = $$MOC_DIR/coreSQLiteStudio
UI_DIR = $$UI_DIR/coreSQLiteStudio

QT       -= gui
QT       += script

TARGET = coreSQLiteStudio
TEMPLATE = lib

win32 {
    INCLUDEPATH += ../../include
    LIBS += -lpsapi
}

LIBS += -lsqlite3

DEFINES += CORESQLITESTUDIO_LIBRARY

QMAKE_CXXFLAGS += -std=c++11 -pedantic

SOURCES += sqlitestudio.cpp \
    returncode.cpp \
    services/config.cpp \
    common/nulldevice.cpp \
    parser/lexer_low_lev.cpp \
    common/utils.cpp \
    parser/keywords.cpp \
    common/utils_sql.cpp \
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
    services/dbmanager.cpp \
    db/sqlresultsrow.cpp \
    db/asyncqueryrunner.cpp \
    db/dbremote.cpp \
    completionhelper.cpp \
    completioncomparer.cpp \
    db/queryexecutor.cpp \
    qio.cpp \
    plugins/pluginsymbolresolver.cpp \
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
    common/readwritelocker.cpp \
    db/queryexecutorsteps/queryexecutorwrapdistinctresults.cpp \
    csvformat.cpp \
    csvserializer.cpp \
    db/queryexecutorsteps/queryexecutordatasources.cpp \
    expectedtoken.cpp \
    sqlhistorymodel.cpp \
    db/queryexecutorsteps/queryexecutorexplainmode.cpp \
    services/notifymanager.cpp \
    parser/statementtokenbuilder.cpp \
    parser/ast/sqlitedeferrable.cpp \
    tablemodifier.cpp \
    db/chainexecutor.cpp \
    db/queryexecutorsteps/queryexecutorreplaceviews.cpp \
    sqlformatter.cpp \
    viewmodifier.cpp \
    log.cpp \
    plugins/plugintype.cpp \
    plugins/genericplugin.cpp \
    common/memoryusage.cpp \
    ddlhistorymodel.cpp \
    datatype.cpp \
    common/table.cpp \
    common/column.cpp \
    dbattacher.cpp \
    services/functionmanager.cpp \
    plugins/scriptingqt.cpp \
    services/impl/configimpl.cpp \
    services/impl/dbmanagerimpl.cpp \
    db/abstractdb.cpp \
    services/impl/functionmanagerimpl.cpp \
    services/impl/pluginmanagerimpl.cpp \
    impl/dbattacherimpl.cpp \
    db/dbsqlite3.cpp \
    plugins/dbpluginsqlite3.cpp \
    parser/ast/sqlitewith.cpp \
    services/impl/collationmanagerimpl.cpp \
    config_builder.cpp \
    services/exportmanager.cpp \
    exportworker.cpp \
    plugins/scriptingsql.cpp \
    db/queryexecutorsteps/queryexecutordetectschemaalter.cpp \
    querymodel.cpp \
    plugins/genericexportplugin.cpp \
    dbobjectorganizer.cpp \
    db/attachguard.cpp \
    db/invaliddb.cpp \
    dbversionconverter.cpp \
    diff/diff_match_patch.cpp \
    db/sqlquery.cpp \
    db/queryexecutorsteps/queryexecutorvaluesmode.cpp \
    services/importmanager.cpp \
    importworker.cpp \
    services/populatemanager.cpp \
    pluginservicebase.cpp \
    populateworker.cpp \
    plugins/populatesequence.cpp

HEADERS += sqlitestudio.h\
        coreSQLiteStudio_global.h \
    returncode.h \
    services/config.h \
    common/nulldevice.h \
    parser/lexer_low_lev.h \
    common/utils.h \
    parser/keywords.h \
    parser/token.h \
    common/utils_sql.h \
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
    common/objectpool.h \
    selectresolver.h \
    schemaresolver.h \
    dialect.h \
    db/db.h \
    services/dbmanager.h \
    db/sqlresultsrow.h \
    db/asyncqueryrunner.h \
    db/dbremote.h \
    completionhelper.h \
    expectedtoken.h \
    completioncomparer.h \
    plugins/dbplugin.h \
    services/pluginmanager.h \
    db/queryexecutor.h \
    qio.h \
    db/dbpluginoption.h \
    common/global.h \
    parser/ast/sqlitetablerelatedddl.h \
    plugins/pluginsymbolresolver.h \
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
    common/unused.h \
    db/queryexecutorsteps/queryexecutororder.h \
    common/readwritelocker.h \
    db/queryexecutorsteps/queryexecutorwrapdistinctresults.h \
    csvformat.h \
    csvserializer.h \
    db/queryexecutorsteps/queryexecutordatasources.h \
    sqlhistorymodel.h \
    db/queryexecutorsteps/queryexecutorexplainmode.h \
    services/notifymanager.h \
    parser/statementtokenbuilder.h \
    tablemodifier.h \
    db/chainexecutor.h \
    db/queryexecutorsteps/queryexecutorreplaceviews.h \
    plugins/sqlformatterplugin.h \
    sqlformatter.h \
    viewmodifier.h \
    log.h \
    plugins/plugintype.h \
    plugins/plugin.h \
    plugins/genericplugin.h \
    common/memoryusage.h \
    ddlhistorymodel.h \
    datatype.h \
    plugins/generalpurposeplugin.h \
    common/table.h \
    common/column.h \
    common/bihash.h \
    common/strhash.h \
    dbattacher.h \
    common/bistrhash.h \
    services/functionmanager.h \
    common/sortedhash.h \
    plugins/scriptingplugin.h \
    plugins/scriptingqt.h \
    services/impl/configimpl.h \
    services/impl/dbmanagerimpl.h \
    db/abstractdb.h \
    services/impl/functionmanagerimpl.h \
    services/impl/pluginmanagerimpl.h \
    impl/dbattacherimpl.h \
    db/abstractdb3.h \
    db/dbsqlite3.h \
    plugins/dbpluginsqlite3.h \
    db/abstractdb2.h \
    parser/ast/sqlitewith.h \
    services/collationmanager.h \
    services/impl/collationmanagerimpl.h \
    plugins/exportplugin.h \
    config_builder.h \
    services/exportmanager.h \
    exportworker.h \
    plugins/scriptingsql.h \
    db/queryexecutorsteps/queryexecutordetectschemaalter.h \
    querymodel.h \
    plugins/genericexportplugin.h \
    dbobjectorganizer.h \
    db/attachguard.h \
    interruptable.h \
    db/invaliddb.h \
    dbversionconverter.h \
    diff/diff_match_patch.h \
    db/sqlquery.h \
    dbobjecttype.h \
    db/queryexecutorsteps/queryexecutorvaluesmode.h \
    plugins/importplugin.h \
    services/importmanager.h \
    importworker.h \
    plugins/populateplugin.h \
    services/populatemanager.h \
    pluginservicebase.h \
    populateworker.h \
    plugins/populatesequence.h

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

FORMS += \
    plugins/populatesequence.ui


for(form, FORMS): copy_file($$form, $$DESTDIR/$$form)
