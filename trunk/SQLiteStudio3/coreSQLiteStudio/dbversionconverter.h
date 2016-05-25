#ifndef DBVERSIONCONVERTER_H
#define DBVERSIONCONVERTER_H

#include "parser/ast/sqlitequery.h"
#include "parser/ast/sqliteorderby.h"
#include <QList>
#include <QStringList>
#include <QPair>
#include <QMutex>

class Db;
class SchemaResolver;
class SqliteCreateTable;
class SqliteCreateTrigger;
class SqliteCreateIndex;
class SqliteCreateView;
class SqliteCreateVirtualTable;
class SqliteIndexedColumn;
class SqliteSelect;
class SqliteDelete;
class SqliteUpdate;
class SqliteInsert;
class SqliteExpr;
class SqliteBeginTrans;

class API_EXPORT DbVersionConverter : public QObject
{
        Q_OBJECT

    public:
        typedef std::function<bool(const QList<QPair<QString,QString>>& diffs)> ConversionConfimFunction;
        typedef std::function<bool(const QSet<QString>& errors)> ConversionErrorsConfimFunction;

        DbVersionConverter();
        virtual ~DbVersionConverter();

        void convert(Dialect from, Dialect to, Db* srcDb, const QString& targetFile, const QString& targetName, ConversionConfimFunction confirmFunc,
                     ConversionErrorsConfimFunction errorsConfirmFunc);
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
        const QSet<QString>& getErrors() const;
        const QList<SqliteQueryPtr>& getConverted() const;
        QStringList getConvertedSqls() const;
        void reset();
        QList<Dialect> getSupportedVersions() const;
        QStringList getSupportedVersionNames() const;

    private:
        struct FullConversionConfig
        {
            Dialect from;
            Dialect to;
            Db* srcDb = nullptr;
            QString targetFile;
            QString targetName;
            ConversionConfimFunction confirmFunc = nullptr;
            ConversionErrorsConfimFunction errorsConfirmFunc = nullptr;
        };

        void fullConvertStep1();
        void fullConvertStep2();
        bool fullConvertCreateObjectsStep1(Db* db, QStringList& tables);
        bool fullConvertCreateObjectsStep2(Db* db);
        bool fullConvertCopyData(Db* db, const QStringList& tables);
        bool checkForInterrupted(Db* db, bool rollback);
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
        bool modifyVirtualTableForVesion2(SqliteQueryPtr& query, SqliteCreateVirtualTable* createVirtualTable);
        bool modifyAllExprsForVersion2(SqliteStatement* stmt);
        bool modifySingleExprForVersion2(SqliteExpr* expr);
        bool modifyAllIndexedColumnsForVersion2(SqliteStatement* stmt);
        bool modifySingleIndexedColumnForVersion2(SqliteExtendedIndexedColumn* idxCol);
        bool modifyBeginTransForVersion3(SqliteBeginTrans* begin);
        bool modifyCreateTableForVersion3(SqliteCreateTable* createTable);
        QString getSqlForDiff(SqliteStatement* stmt);
        void storeDiff(const QString& sql1, SqliteStatement* stmt);
        void storeErrorDiff(SqliteStatement* stmt);
        QList<Db*> getAllPossibleDbInstances() const;
        QString generateQueryPlaceholders(int argCount);
        void sortConverted();
        void setInterrupted(bool value);
        bool isInterrupted();
        void conversionInterrupted(Db* db, bool rollback);

        template <class T>
        bool modifyAllIndexedColumnsForVersion2(const QList<T*> columns)
        {
            for (T* idxCol : columns)
            {
                if (!modifySingleIndexedColumnForVersion2(idxCol))
                    return false;
            }
            return true;
        }


        template <class T>
        QSharedPointer<T> copyQuery(SqliteQueryPtr query)
        {
            return QSharedPointer<T>::create(*(query.dynamicCast<T>().data()));
        }

        Db* db = nullptr;
        Dialect targetDialect = Dialect::Sqlite3;
        SchemaResolver* resolver = nullptr;
        QList<QPair<QString,QString>> diffList;
        QSet<QString> errors;
        QList<SqliteQueryPtr> newQueries;
        FullConversionConfig* fullConversionConfig = nullptr;
        bool interrupted = false;
        QMutex interruptMutex;

    private slots:
        void conversionError(Db* db, const QString& errMsg);
        void confirmConversion();
        void registerDbAfterSuccessfulConversion();

    public slots:
        void interrupt();

    signals:
        void askUserForConfirmation();
        void conversionSuccessful();
        void conversionAborted();
        void conversionFailed(const QString& errorMsg);
};

#endif // DBVERSIONCONVERTER_H
