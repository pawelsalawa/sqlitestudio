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
    }
    return QString();
}

bool isDataReturningQuery(const SqliteQueryType& type)
{
    switch (type)
    {
        case SqliteQueryType::Select:
        case SqliteQueryType::Pragma:
            return true;
        case SqliteQueryType::UNDEFINED:
        case SqliteQueryType::EMPTY:
        case SqliteQueryType::AlterTable:
        case SqliteQueryType::Analyze:
        case SqliteQueryType::Attach:
        case SqliteQueryType::BeginTrans:
        case SqliteQueryType::CommitTrans:
        case SqliteQueryType::Copy:
        case SqliteQueryType::CreateIndex:
        case SqliteQueryType::CreateTable:
        case SqliteQueryType::CreateTrigger:
        case SqliteQueryType::CreateView:
        case SqliteQueryType::CreateVirtualTable:
        case SqliteQueryType::Delete:
        case SqliteQueryType::Detach:
        case SqliteQueryType::DropIndex:
        case SqliteQueryType::DropTable:
        case SqliteQueryType::DropTrigger:
        case SqliteQueryType::DropView:
        case SqliteQueryType::Insert:
        case SqliteQueryType::Reindex:
        case SqliteQueryType::Release:
        case SqliteQueryType::Rollback:
        case SqliteQueryType::Savepoint:
        case SqliteQueryType::Update:
        case SqliteQueryType::Vacuum:
            return false;
    }
    return false;
}
