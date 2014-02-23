#ifndef VIEWMODIFIER_H
#define VIEWMODIFIER_H

#include "db/db.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include <QString>

class API_EXPORT ViewModifier
{
    public:
        ViewModifier(Db* db, const QString& view);
        ViewModifier(Db* db, const QString& database, const QString& view);

        void alterView(const QString& newView);
        void alterView(SqliteCreateViewPtr newView);

        QStringList generateSqls() const;
        QList<bool> getMandatoryFlags() const;
        QStringList getWarnings() const;
        QStringList getErrors() const;
        bool hasMessages() const;

    private:
        void handleTriggers();
        void collectNewColumns();
        void addMandatorySql(const QString& sql);
        void addOptionalSql(const QString& sql);
        bool handleNewColumns(SqliteCreateTriggerPtr trigger);

        Db* db = nullptr;
        Dialect dialect;
        QString database;
        QString view;

        /**
         * @brief sqls Statements to be executed to make changes real.
         */
        QStringList sqls;
        QList<bool> sqlMandatoryFlags;

        QStringList warnings;
        QStringList errors;

        /**
         * @brief createView Original DDL.
         */
        SqliteCreateViewPtr createView;

        QStringList newColumns;
};

#endif // VIEWMODIFIER_H
