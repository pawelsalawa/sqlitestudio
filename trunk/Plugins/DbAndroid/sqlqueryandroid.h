#ifndef SQLQUERYANDROID_H
#define SQLQUERYANDROID_H

#include "db/sqlquery.h"
#include "parser/token.h"
#include <QJsonDocument>

class DbAndroidConnection;
class DbAndroidInstance;

class SqlQueryAndroid : public SqlQuery
{
    public:
        SqlQueryAndroid(DbAndroidInstance* db, DbAndroidConnection* connection, const QString& query);
        ~SqlQueryAndroid();

        QString getErrorText();
        int getErrorCode();
        QStringList getColumnNames();
        int columnCount();
        void rewind();

    protected:
        SqlResultsRowPtr nextInternal();
        bool hasNextInternal();
        bool execInternal(const QList<QVariant>& args);
        bool execInternal(const QHash<QString, QVariant>& args);

    private:
        bool executeAndHandleResponse(const QString& query);
        void resetResponse();

        static QString convertArg(const QVariant& value);

        DbAndroidInstance* db = nullptr;
        DbAndroidConnection* connection = nullptr;
        QString queryString;
        TokenList tokenizedQuery;
        int errorCode = 0;
        QString errorText;
        QStringList resultColumns;
        QList<QVariantHash> resultDataMap;
        QList<QVariantList> resultDataList;
        int currentRow = -1;
};

#endif // SQLQUERYANDROID_H
