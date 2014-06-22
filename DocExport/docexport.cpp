#include "docexport.h"
#include "common/global.h"
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>

DocExport::DocExport()
{
}

void DocExport::newDoc()
{
    cleanupDoc();
    doc = new QTextDocument();
    cursor = new QTextCursor(doc);
}

void DocExport::endDoc()
{

}

void DocExport::addTable(SqliteCreateTablePtr createTable)
{
    docTable = cursor->insertTable(1, 1);
    docTable->cellAt(0, 0).firstCursorPosition().insertText(createTable->table);
}

void DocExport::addTableRow(SqlResultsRowPtr row)
{

}

void DocExport::addVirtualTable(SqliteCreateVirtualTablePtr createVirtualTable)
{

}

void DocExport::addIndex(SqliteCreateIndexPtr createIndex)
{

}

void DocExport::addTrigger(SqliteCreateTriggerPtr createTrigger)
{

}

void DocExport::addView(SqliteCreateViewPtr createView)
{

}

QTextDocument* DocExport::getDocument() const
{
    return doc;
}

void DocExport::cleanupDoc()
{
    safe_delete(doc);
    safe_delete(cursor);
}

bool DocExport::init()
{
    return false;
}
