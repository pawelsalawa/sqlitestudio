#include "queryexecutorvaluesmode.h"

bool QueryExecutorValuesMode::exec()
{
    SqliteSelectPtr select = getSelect();
    if (!select || select->explain)
        return true;

    bool modified = false;
    for (SqliteSelect::Core* core : select->coreSelects)
    {
        if (core->valuesMode)
        {
            core->valuesMode = false;
            modified = true;
        }
    }

    if (modified)
    {
        select->rebuildTokens();
        updateQueries();
    }

    return true;
}
