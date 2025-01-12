#ifndef SQLITEANALYZE_H
#define SQLITEANALYZE_H

#include "sqlitequery.h"
#include <QString>

class API_EXPORT SqliteAnalyze : public SqliteQuery
{
    Q_OBJECT

    public:
        SqliteAnalyze();
        SqliteAnalyze(const SqliteAnalyze& other);
        SqliteAnalyze(const QString& name1, const QString& name2);
        SqliteStatement* clone();

        QString database = QString();
        QString table = QString();

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteAnalyze> SqliteAnalyzePtr;

#endif // SQLITEANALYZE_H
