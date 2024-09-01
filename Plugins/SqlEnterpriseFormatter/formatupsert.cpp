#include "formatupsert.h"
#include "parser/ast/sqliteupsert.h"
#include "parser/ast/sqliteexpr.h"
#include "parser/ast/sqliteorderby.h"

FormatUpsert::FormatUpsert(SqliteUpsert* upsert) :
    upsert(upsert)
{

}

void FormatUpsert::formatInternal()
{
    withKeyword("ON").withKeyword("CONFLICT");
    if (!upsert->conflictColumns.isEmpty())
    {
        withParDefLeft().withStatementList(upsert->conflictColumns).withParDefRight();
        if (upsert->conflictWhere)
            withKeyword("WHERE").withStatement(upsert->conflictWhere);
    }

    withKeyword("DO");

    if (upsert->doNothing)
    {
        withKeyword("NOTHING");
    }
    else
    {
        withKeyword("UPDATE").withKeyword("SET");
        bool first = true;
        for (const SqliteUpsert::ColumnAndValue& keyVal : upsert->keyValueMap)
        {
            if (!first)
                withListComma();

            if (keyVal.first.userType() == QMetaType::QStringList)
                withParDefLeft().withIdList(keyVal.first.toStringList()).withParDefRight().withOperator("=").withStatement(keyVal.second);
            else
                withId(keyVal.first.toString()).withOperator("=").withStatement(keyVal.second);

            first = false;
        }

        if (upsert->setWhere)
            withKeyword("WHERE").withStatement(upsert->setWhere);
    }

}
