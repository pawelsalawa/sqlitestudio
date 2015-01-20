#include "queryexecutordetectschemaalter.h"

bool QueryExecutorDetectSchemaAlter::exec()
{
    for (SqliteQueryPtr query : context->parsedQueries)
    {
        switch (query->queryType)
        {
            case SqliteQueryType::AlterTable:
            case SqliteQueryType::CreateIndex:
            case SqliteQueryType::CreateTable:
            case SqliteQueryType::CreateTrigger:
            case SqliteQueryType::CreateView:
            case SqliteQueryType::DropIndex:
            case SqliteQueryType::DropTable:
            case SqliteQueryType::DropTrigger:
            case SqliteQueryType::DropView:
            case SqliteQueryType::CreateVirtualTable:
                context->schemaModified = true;
                break;
            case SqliteQueryType::Insert:
            case SqliteQueryType::Delete:
            case SqliteQueryType::Update:
                context->dataModifyingQuery = true;
            default:
                break;
        }
    }
    return true;
}
