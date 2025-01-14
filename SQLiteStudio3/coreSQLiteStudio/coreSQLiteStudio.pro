#-------------------------------------------------
#
# Project created by QtCreator 2013-02-28T23:00:28
#
#-------------------------------------------------

include($$PWD/../common.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/coreSQLiteStudio
MOC_DIR = $$MOC_DIR/coreSQLiteStudio
UI_DIR = $$UI_DIR/coreSQLiteStudio

QT       -= gui
QT       += qml network

TARGET = coreSQLiteStudio
TEMPLATE = lib

win32: {
    LIBS += -lpsapi -limagehlp -lversion
    DEFINES += "SQLITE_API=\"__declspec(dllexport)\""

    !debug: {
        THE_FILE = $$PWD/qt.conf
        THE_DEST = $${DESTDIR}
        THE_FILE ~= s,/,\\,g
        THE_DEST ~= s,/,\\,g
        QMAKE_POST_LINK += "$$QMAKE_COPY $$THE_FILE $$THE_DEST $$escape_expand(\\n\\t);"
    }
}

linux: {
    DEFINES += SYS_PLUGINS_DIR=$$LIBDIR/sqlitestudio
    portable: {
        DESTDIR = $$DESTDIR/lib
    }
}

macx: {
    out_file = $$DESTDIR/lib $$TARGET .dylib
    QMAKE_POST_LINK += "install_name_tool -change libsqlite3.dylib @loader_path/../Frameworks/libsqlite3.dylib \"$$join(out_file)\""
    QMAKE_POST_LINK += "; $$QMAKE_MKDIR \"$$DESTDIR/SQLiteStudio.app\""
    QMAKE_POST_LINK += "; $$QMAKE_MKDIR \"$$DESTDIR/SQLiteStudio.app/Contents\""
    QMAKE_POST_LINK += "; $$QMAKE_COPY \"$$PWD/Info.plist\" \"$$DESTDIR/SQLiteStudio.app/Contents\""
    LIBS += -L/usr/local/lib
}

LIBS += -lsqlite3

DEFINES += CORESQLITESTUDIO_LIBRARY

portable {
    DEFINES += PORTABLE_CONFIG
}

CONFIG  += c++20
QMAKE_CXXFLAGS += -pedantic

SOURCES += sqlitestudio.cpp \
    chillout/chillout.cpp \
    chillout/common/common.cpp \
    chillout/posix/posixcrashhandler.cpp \
    chillout/windows/StackWalker.cpp \
    chillout/windows/windowscrashhandler.cpp \
    db/queryexecutorsteps/queryexecutorcolumntype.cpp \
    db/queryexecutorsteps/queryexecutorfilter.cpp \
    db/queryexecutorsteps/queryexecutorsmarthints.cpp \
    parser/ast/sqlitefilterover.cpp \
    parser/ast/sqlitenulls.cpp \
    parser/ast/sqlitewindowdefinition.cpp \
    returncode.cpp \
    services/codesnippetmanager.cpp \
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
    db/queryexecutorsteps/queryexecutororder.cpp \
    db/sqlerrorcodes.cpp \
    common/readwritelocker.cpp \
    db/queryexecutorsteps/queryexecutorwrapdistinctresults.cpp \
    csvformat.cpp \
    csvserializer.cpp \
    db/queryexecutorsteps/queryexecutordatasources.cpp \
    expectedtoken.cpp \
    sqlfileexecutor.cpp \
    sqlhistorymodel.cpp \
    db/queryexecutorsteps/queryexecutorexplainmode.cpp \
    services/notifymanager.cpp \
    parser/statementtokenbuilder.cpp \
    parser/ast/sqlitedeferrable.cpp \
    tablemodifier.cpp \
    db/chainexecutor.cpp \
    db/queryexecutorsteps/queryexecutorreplaceviews.cpp \
    services/codeformatter.cpp \
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
    services/exportmanager.cpp \
    exportworker.cpp \
    plugins/scriptingsql.cpp \
    db/queryexecutorsteps/queryexecutordetectschemaalter.cpp \
    querymodel.cpp \
    plugins/genericexportplugin.cpp \
    dbobjectorganizer.cpp \
    db/attachguard.cpp \
    db/invaliddb.cpp \
    diff/diff_match_patch.cpp \
    db/sqlquery.cpp \
    db/queryexecutorsteps/queryexecutorvaluesmode.cpp \
    services/importmanager.cpp \
    importworker.cpp \
    services/populatemanager.cpp \
    pluginservicebase.cpp \
    populateworker.cpp \
    plugins/populatesequence.cpp \
    plugins/populaterandom.cpp \
    plugins/populaterandomtext.cpp \
    plugins/populateconstant.cpp \
    plugins/populatedictionary.cpp \
    plugins/populatescript.cpp \
    plugins/builtinplugin.cpp \
    plugins/scriptingqtdbproxy.cpp \
    plugins/sqlformatterplugin.cpp \
    services/updatemanager.cpp \
    config_builder/cfgmain.cpp \
    config_builder/cfgcategory.cpp \
    config_builder/cfgentry.cpp \
    config_builder/cfglazyinitializer.cpp \
    committable.cpp \
    services/extralicensemanager.cpp \
    tsvserializer.cpp \
    rsa/BigInt.cpp \
    rsa/Key.cpp \
    rsa/KeyPair.cpp \
    rsa/PrimeGenerator.cpp \
    rsa/RSA.cpp \
    translations.cpp \
    common/signalwait.cpp \
    common/blockingsocket.cpp \
    common/threadwitheventloop.cpp \
    common/private/blockingsocketprivate.cpp \
    querygenerator.cpp \
    common/bistrhash.cpp \
    plugins/dbpluginstdfilebase.cpp \
    common/xmldeserializer.cpp \
    services/impl/sqliteextensionmanagerimpl.cpp \
    common/lazytrigger.cpp \
    parser/ast/sqliteupsert.cpp

HEADERS += sqlitestudio.h\
    chillout/chillout.h \
    chillout/common/common.h \
    chillout/defines.h \
    chillout/posix/posixcrashhandler.h \
    chillout/windows/StackWalker.h \
    chillout/windows/windowscrashhandler.h \
        coreSQLiteStudio_global.h \
    db/queryexecutorsteps/queryexecutorcolumntype.h \
    db/queryexecutorsteps/queryexecutorfilter.h \
    db/queryexecutorsteps/queryexecutorsmarthints.h \
    db/sqlite3.h \
    parser/ast/sqlitefilterover.h \
    parser/ast/sqlitenulls.h \
    parser/ast/sqlitequerywithaliasedtable.h \
    parser/ast/sqlitewindowdefinition.h \
    returncode.h \
    services/codesnippetmanager.h \
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
    db/db.h \
    services/dbmanager.h \
    db/sqlresultsrow.h \
    db/asyncqueryrunner.h \
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
    common/unused.h \
    db/queryexecutorsteps/queryexecutororder.h \
    common/readwritelocker.h \
    db/queryexecutorsteps/queryexecutorwrapdistinctresults.h \
    csvformat.h \
    csvserializer.h \
    db/queryexecutorsteps/queryexecutordatasources.h \
    sqlfileexecutor.h \
    sqlhistorymodel.h \
    db/queryexecutorsteps/queryexecutorexplainmode.h \
    services/notifymanager.h \
    parser/statementtokenbuilder.h \
    tablemodifier.h \
    db/chainexecutor.h \
    db/queryexecutorsteps/queryexecutorreplaceviews.h \
    plugins/sqlformatterplugin.h \
    services/codeformatter.h \
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
    plugins/populatesequence.h \
    plugins/populaterandom.h \
    plugins/populaterandomtext.h \
    plugins/populateconstant.h \
    plugins/populatedictionary.h \
    plugins/populatescript.h \
    plugins/builtinplugin.h \
    plugins/scriptingqtdbproxy.h \
    plugins/codeformatterplugin.h \
    services/updatemanager.h \
    config_builder/cfgmain.h \
    config_builder/cfgcategory.h \
    config_builder/cfgentry.h \
    config_builder/cfglazyinitializer.h \
    plugins/confignotifiableplugin.h \
    committable.h \
    plugins/uiconfiguredplugin.h \
    services/extralicensemanager.h \
    db/stdsqlite3driver.h \
    tsvserializer.h \
    rsa/BigInt.h \
    rsa/Key.h \
    rsa/KeyPair.h \
    rsa/PrimeGenerator.h \
    rsa/RSA.h \
    translations.h \
    common/signalwait.h \
    common/blockingsocket.h \
    common/threadwitheventloop.h \
    common/private/blockingsocketprivate.h \
    common/expiringcache.h \
    parser/ast/sqliteddlwithdbcontext.h \
    parser/ast/sqliteextendedindexedcolumn.h \
    querygenerator.h \
    common/sortedset.h \
    plugins/dbpluginstdfilebase.h \
    common/xmldeserializer.h \
    common/valuelocker.h \
    services/sqliteextensionmanager.h \
    services/impl/sqliteextensionmanagerimpl.h \
    common/lazytrigger.h \
    parser/ast/sqliteupsert.h

unix: {
    target.path = $$LIBDIR
    INSTALLS += target
}

OTHER_FILES += \
    parser/lempar.c \
    parser/lemon.c \
    parser/sqlite3_parse.y \
    parser/run_lemon.sh \
    licenses/fugue_icons.txt \
    licenses/qhexedit.txt \
    licenses/sqlitestudio_license.txt \
    licenses/lgpl.txt \
    licenses/diff_match.txt \
    licenses/gpl.txt \
    qt.conf
 \
    Info.plist

FORMS += \
    plugins/populatesequence.ui \
    plugins/populaterandom.ui \
    plugins/populaterandomtext.ui \
    plugins/populateconstant.ui \
    plugins/populatedictionary.ui \
    plugins/populatescript.ui

RESOURCES += \
    coreSQLiteStudio.qrc

DISTFILES += \
    licenses/icons.txt \
    licenses/icu.txt \
    licenses/mit.txt
