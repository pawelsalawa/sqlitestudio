#ifndef SQLITEANALYZE_H
#define SQLITEANALYZE_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteAnalyze : public SqliteQuery
{
    public:
        SqliteAnalyze();
        SqliteAnalyze(const QString& name1, const QString& name2);

        QString database = QString::null;
        QString table = QString::null;

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
};

typedef QSharedPointer<SqliteAnalyze> SqliteAnalyzePtr;

#endif // SQLITEANALYZE_H
