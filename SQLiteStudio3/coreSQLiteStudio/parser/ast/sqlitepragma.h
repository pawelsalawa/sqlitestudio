#ifndef SQLITEPRAGMA_H
#define SQLITEPRAGMA_H

#include "sqlitequery.h"

#include <QString>
#include <QVariant>

class API_EXPORT SqlitePragma : public SqliteQuery
{
    Q_OBJECT

    public:
        SqlitePragma();
        SqlitePragma(const SqlitePragma& other);
        SqlitePragma(const QString& name1, const QString& name2);
        SqlitePragma(const QString& name1, const QString& name2, const QVariant& value,
                     bool equals);
        SqlitePragma(const QString& name1, const QString& name2, const QString& value,
                     bool equals);

        SqliteStatement* clone();

    protected:
        QStringList getDatabasesInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        void initName(const QString& name1, const QString& name2);

    public:
        QString database = QString();
        QString pragmaName = QString();
        QVariant value = QVariant();
        bool equalsOp = false;
        bool parenthesis = false;
};

typedef QSharedPointer<SqlitePragma> SqlitePragmaPtr;

#endif // SQLITEPRAGMA_H
