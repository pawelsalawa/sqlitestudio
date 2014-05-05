#ifndef DBVERSIONCONVERTER_H
#define DBVERSIONCONVERTER_H

#include "parser/ast/sqlitequery.h"
#include <QList>
#include <QStringList>
#include <QPair>

/*
Major 2vs3 diffs:
- references action hasn't got "NO ACTION" as possible action in v2.
 */

class Db;
class SchemaResolver;
class SqliteCreateTable;
class SqliteCreateTrigger;
class SqliteCreateIndex;
class SqliteCreateView;
class SqliteIndexedColumn;
class SqliteSelect;
class SqliteDelete;
class SqliteUpdate;
class SqliteInsert;
class SqliteExpr;
class SqliteBeginTrans;

class DbVersionConverter
{
    public:
        DbVersionConverter();
        virtual ~DbVersionConverter();

        void convertToVersion2(Db* db);
        void convertToVersion3(Db* db);
        QString convertToVersion2(const QString& sql);
        QString convertToVersion3(const QString& sql);
        SqliteQueryPtr convertToVersion2(SqliteQueryPtr query);
        SqliteQueryPtr convertToVersion3(SqliteQueryPtr query);

        const QList<QPair<QString, QString> >& getDiffList() const;
        const QStringList& getErrors() const;
        const QList<SqliteQueryPtr>& getConverted() const;
        QStringList getConvertedSqls() const;
        void reset();

    private:
        void convertDb();
        QList<SqliteQueryPtr> parse(const QString& sql, Dialect dialect);
        bool modifySelectForVersion2(SqliteSelect* select);
        bool modifyDeleteForVersion2(SqliteDelete* del);
        bool modifyInsertForVersion2(SqliteInsert* insert);
        bool modifyUpdateForVersion2(SqliteUpdate* update);
        bool modifyCreateTableForVersion2(SqliteCreateTable* createTable);
        bool modifyCreateTriggerForVersion2(SqliteCreateTrigger* createTrigger);
        bool modifyCreateIndexForVersion2(SqliteCreateIndex* createIndex);
        bool modifyCreateViewForVersion2(SqliteCreateView* createView);
        bool modifyAllExprsForVersion2(SqliteStatement* stmt);
        bool modifySingleExprForVersion2(SqliteExpr* expr);
        bool modifyAllIndexedColumnsForVersion2(SqliteStatement* stmt);
        bool modifyAllIndexedColumnsForVersion2(const QList<SqliteIndexedColumn*> columns);
        bool modifySingleIndexedColumnForVersion2(SqliteIndexedColumn* idxCol);
        bool modifyBeginTransForVersion3(SqliteBeginTrans* begin);
        bool modifyCreateTableForVersion3(SqliteCreateTable* createTable);
        QString getSqlForDiff(SqliteStatement* stmt);
        void storeDiff(const QString& sql1, SqliteStatement* stmt);
        void storeErrorDiff(SqliteStatement* stmt);

        template <class T>
        QSharedPointer<T> copyQuery(SqliteQueryPtr query)
        {
            return QSharedPointer<T>::create(*(query.dynamicCast<T>().data()));
        }

        Db* db = nullptr;
        Dialect targetDialect = Dialect::Sqlite3;
        SchemaResolver* resolver = nullptr;
        QList<QPair<QString,QString>> diffList;
        QStringList errors;
        QList<SqliteQueryPtr> newQueries;
};

#endif // DBVERSIONCONVERTER_H
