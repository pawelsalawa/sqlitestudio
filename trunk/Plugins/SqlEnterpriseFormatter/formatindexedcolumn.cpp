#include "formatindexedcolumn.h"
#include "parser/ast/sqliteindexedcolumn.h"

FormatIndexedColumn::FormatIndexedColumn(SqliteIndexedColumn* idxCol) :
    idxCol(idxCol)
{
}


void FormatIndexedColumn::formatInternal()
{
    withId(idxCol->name);
    if (!idxCol->collate.isNull())
        withKeyword("COLLATE").withId(idxCol->collate);

    withSortOrder(idxCol->sortOrder);
}
