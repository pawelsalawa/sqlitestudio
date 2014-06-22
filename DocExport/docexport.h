#ifndef DOCEXPORT_H
#define DOCEXPORT_H

#include "docexport_global.h"
#include "plugins/generalpurposeplugin.h"
#include "plugins/genericplugin.h"
#include "db/sqlresultsrow.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include <QObject>

class QTextDocument;
class QTextTable;
class QTextCursor;

class DOCEXPORTSHARED_EXPORT DocExport : virtual public GenericPlugin, public GeneralPurposePlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("docexport.json")

    public:
        DocExport();

        void newDoc();
        void endDoc();
        void addTable(SqliteCreateTablePtr createTable);
        void addTableRow(SqlResultsRowPtr row);
        void addVirtualTable(SqliteCreateVirtualTablePtr createVirtualTable);
        void addIndex(SqliteCreateIndexPtr createIndex);
        void addTrigger(SqliteCreateTriggerPtr createTrigger);
        void addView(SqliteCreateViewPtr createView);
        QTextDocument* getDocument() const;
        void cleanupDoc();

    private:
        QTextDocument* doc = nullptr;
        QTextTable* docTable = nullptr;
        QTextCursor* cursor = nullptr;

        // Plugin interface
    public:
        bool init();
};

#endif // DOCEXPORT_H
