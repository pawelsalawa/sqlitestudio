#ifndef ERDCHANGEDELETECONNECTION_H
#define ERDCHANGEDELETECONNECTION_H

#include "erdchange.h"
#include "parser/ast/sqlitecreatetable.h"

class TableModifier;
class Db;
class ErdConnection;

class ErdChangeDeleteConnection : public ErdChange
{
    public:
        ErdChangeDeleteConnection(Db* db, ErdConnection* connection, const QString& description);
        ~ErdChangeDeleteConnection();

        TableModifier *getTableModifier() const;
        QString getStartEntityName() const;

    protected:
        QStringList getChangeDdl();

    private:
        Db* db = nullptr;
        SqliteCreateTablePtr createTable;
        QString startEntityName;
        QString endEntityName;
        QList<QPair<QString, QString>> columnPairs;
        TableModifier* tableModifier = nullptr;
};

#endif // ERDCHANGEDELETECONNECTION_H
