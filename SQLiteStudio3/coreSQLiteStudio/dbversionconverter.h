#ifndef DBVERSIONCONVERTER_H
#define DBVERSIONCONVERTER_H

#include "parser/ast/sqlitequery.h"
#include <QList>
#include <QStringList>
#include <QPair>

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

class DbVersionConverter : public QObject
{
        Q_OBJECT
        Q_ENUMS(Version)

    public:
        DbVersionConverter();
        virtual ~DbVersionConverter();

        void convert(Dialect from, Dialect to, Db* db);
        void convert3To2(Db* db);
        void convert2To3(Db* db);
        QString convert(Dialect from, Dialect to, const QString& sql);
        QString convert3To2(const QString& sql);
        QString convert2To3(const QString& sql);
        SqliteQueryPtr convert(Dialect from, Dialect to, SqliteQueryPtr query);
        SqliteQueryPtr convert3To2(SqliteQueryPtr query);
        SqliteQueryPtr convert2To3(SqliteQueryPtr query);

        const QList<QPair<QString, QString> >& getDiffList() const;
        const QStringList& getErrors() const;
        const QList<SqliteQueryPtr>& getConverted() const;
        QStringList getConvertedSqls() const;
        void reset();
        QList<Dialect> getSupportedVersions() const;
        QStringList getSupportedVersionNames() const;

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
        QList<Db*> getAllPossibleDbInstances() const;

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
