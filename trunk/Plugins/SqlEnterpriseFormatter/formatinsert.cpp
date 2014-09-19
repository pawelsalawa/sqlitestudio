#include "formatinsert.h"
#include "parser/ast/sqliteselect.h"
#include "parser/ast/sqliteinsert.h"
#include "formatwith.h"

FormatInsert::FormatInsert(SqliteInsert* insert) :
    insert(insert)
{
}

void FormatInsert::formatInternal()
{
    if (insert->replaceKw)
    {
        withStatement(insert->with, QString(), [](FormatStatement* stmt)
        {
            dynamic_cast<FormatWith*>(stmt)->setLineUpKeyword("REPLACE");
        });

        withKeyword("REPLACE");
    }
    else
    {
        withStatement(insert->with, QString(), [](FormatStatement* stmt)
        {
            dynamic_cast<FormatWith*>(stmt)->setLineUpKeyword("INSERT");
        });

        withKeyword("INSERT");
        if (insert->onConflict != SqliteConflictAlgo::null)
            withKeyword("OR").withKeyword(sqliteConflictAlgo(insert->onConflict));
    }

    withKeyword("INTO");

    if (!insert->database.isNull())
        withId(insert->database);

    withId(insert->table);

    if (insert->defaultValuesKw)
    {
        withKeyword("DEFAULT").withKeyword("VALUES");
    }
    else
    {
        markAndKeepIndent("insertCols");
        if (insert->columnNames.size() > 0)
            withParDefLeft().withIdList(insert->columnNames).withParDefRight();

        if (insert->select)
        {
            withStatement(insert->select);
        }
        else if (dialect == Dialect::Sqlite2) // Sqlite2 uses classic single row values
        {
            withKeyword("VALUES").withParDefLeft().withStatementList(insert->values).withParDefRight();
        }
        withDecrIndent();
    }
    withSemicolon();
}
