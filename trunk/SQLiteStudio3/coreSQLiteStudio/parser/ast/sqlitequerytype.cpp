#include "sqlitequerytype.h"

QString sqliteQueryTypeToString(const SqliteQueryType& type)
{
    switch (type)
    {
        case SqliteQueryType::UNDEFINED:
            return "UNDEFINED";
        case SqliteQueryType::EMPTY:
            return "EMPTY";
        case SqliteQueryType::AlterTable:
            return "AlterTable";
        case SqliteQueryType::Analyze:
            return "Analyze";
        case SqliteQueryType::Attach:
            return "Attach";
        case SqliteQueryType::BeginTrans:
            return "BeginTrans";
        case SqliteQueryType::CommitTrans:
            return "CommitTrans";
        case SqliteQueryType::Copy:
            return "Copy";
        case SqliteQueryType::CreateIndex:
            return "CreateIndex";
        case SqliteQueryType::CreateTable:
            return "CreateTable";
        case SqliteQueryType::CreateTrigger:
            return "CreateTrigger";
        case SqliteQueryType::CreateView:
            return "CreateView";
        case SqliteQueryType::CreateVirtualTable:
            return "CreateVirtualTable";
        case SqliteQueryType::Delete:
            return "Delete";
        case SqliteQueryType::Detach:
            return "Detach";
        case SqliteQueryType::DropIndex:
            return "DropIndex";
        case SqliteQueryType::DropTable:
            return "DropTable";
        case SqliteQueryType::DropTrigger:
            return "DropTrigger";
        case SqliteQueryType::DropView:
            return "DropView";
        case SqliteQueryType::Insert:
            return "Insert";
        case SqliteQueryType::Pragma:
            return "Pragma";
        case SqliteQueryType::Reindex:
            return "Reindex";
        case SqliteQueryType::Release:
            return "Release";
        case SqliteQueryType::Rollback:
            return "Rollback";
        case SqliteQueryType::Savepoint:
            return "Savepoint";
        case SqliteQueryType::Select:
            return "Select";
        case SqliteQueryType::Update:
            return "Update";
        case SqliteQueryType::Vacuum:
            return "Vacuum";
        default:
            return QString::null;
    }
}
