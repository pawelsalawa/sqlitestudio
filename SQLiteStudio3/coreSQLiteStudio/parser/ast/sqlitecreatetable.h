#ifndef SQLITECREATETABLE_H
#define SQLITECREATETABLE_H

#include "sqlitequery.h"
#include "sqliteconflictalgo.h"
#include "sqliteexpr.h"
#include "sqliteforeignkey.h"
#include "sqliteindexedcolumn.h"
#include "sqliteselect.h"
#include "sqlitecolumntype.h"
#include "sqlitesortorder.h"
#include "sqlitedeferrable.h"
#include "sqliteddlwithdbcontext.h"
#include <QVariant>
#include <QList>
#include <QRegularExpression>

struct ParserStubCreateTableOption;

class API_EXPORT SqliteCreateTable : public SqliteQuery, public SqliteDdlWithDbContext
{
    public:
        class API_EXPORT Column : public SqliteStatement
        {
            public:
                class API_EXPORT Constraint : public SqliteStatement
                {
                    friend class Column;

                    public:
                        enum Type
                        {
                            PRIMARY_KEY,
                            NOT_NULL,
                            UNIQUE,
                            CHECK,
                            DEFAULT,
                            COLLATE,
                            FOREIGN_KEY,
                            GENERATED,
                            NULL_,           // not officially supported
                            NAME_ONLY,      // unofficial, because of bizarre sqlite grammar
                            DEFERRABLE_ONLY // unofficial, because of bizarre sqlite grammar
                        };

                        enum class GeneratedType
                        {
                            STORED,
                            VIRTUAL,
                            null
                        };

                        Constraint();
                        Constraint(const Constraint& other);
                        ~Constraint();
                        SqliteStatement* clone();

                        static QString toString(GeneratedType type);
                        static GeneratedType generatedTypeFrom(const QString& type);

                        void initDefNameOnly(const QString& name);
                        void initDefId(const QString& id);
                        void initDefTerm(const QVariant& value, bool minus = false);
                        void initDefCTime(const QString& name);
                        void initDefExpr(SqliteExpr* expr);
                        void initNull(SqliteConflictAlgo algo);
                        void initNotNull(SqliteConflictAlgo algo);
                        void initPk(SqliteSortOrder order, SqliteConflictAlgo algo, bool autoincr);
                        void initUnique(SqliteConflictAlgo algo);
                        void initCheck();
                        void initCheck(SqliteExpr* expr);
                        void initCheck(SqliteExpr* expr, SqliteConflictAlgo algo);
                        void initFk(const QString& table, const QList<SqliteIndexedColumn*>& indexedColumns, const QList<SqliteForeignKey::Condition*>& conditions);
                        void initDefer(SqliteInitially initially, SqliteDeferrable deferrable);
                        void initColl(const QString& name);
                        void initGeneratedAs(SqliteExpr* expr, bool genKw, const QString& type);
                        QString typeString() const;

                        Type type;
                        QString name = QString();
                        SqliteSortOrder sortOrder = SqliteSortOrder::null;
                        SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
                        bool autoincrKw = false;
                        bool generatedKw = false;
                        SqliteExpr* expr = nullptr;
                        QVariant literalValue;
                        bool literalNull = false;
                        QString ctime;
                        QString id;
                        QString collationName = QString();
                        GeneratedType generatedType = GeneratedType::null;
                        SqliteForeignKey* foreignKey = nullptr;

                    protected:
                        TokenList rebuildTokensFromContents();

                    private:
                        SqliteDeferrable deferrable = SqliteDeferrable::null; // only a temporary field for parse time, before merging with actual FK
                        SqliteInitially initially = SqliteInitially::null; // only a temporary field for parse time, before merging with actual FK

                };

                typedef QSharedPointer<Constraint> ConstraintPtr;

                Column();
                Column(const Column& other);
                Column(const QString& name, SqliteColumnType* type,
                       const QList<Constraint*>& constraints);
                ~Column();
                SqliteStatement* clone();

                bool hasConstraint(Constraint::Type type) const;
                Constraint* getConstraint(Constraint::Type type) const;
                QList<Constraint*> getConstraints(Constraint::Type type) const;
                QList<Constraint*> getForeignKeysByTable(const QString& foreignTable) const;
                void fixTypeVsGeneratedAs();
                void evaluatePostParsing();

                QString name = QString();
                SqliteColumnType* type = nullptr;
                QList<Constraint*> constraints;

                /**
                 * @brief originalName
                 * Used to remember original name when column was edited and the name was changed.
                 * It's defined in the constructor to the same value as the name member.
                 */
                QString originalName = QString();

            protected:
                QStringList getColumnsInStatement();
                TokenList getColumnTokensInStatement();
                TokenList rebuildTokensFromContents();

            private:
                static const QRegularExpression GENERATED_ALWAYS_REGEXP;
        };

        typedef QSharedPointer<Column> ColumnPtr;

        class API_EXPORT Constraint : public SqliteStatement
        {
            public:
                enum Type
                {
                    PRIMARY_KEY,
                    UNIQUE,
                    CHECK,
                    FOREIGN_KEY,
                    NAME_ONLY       // unofficial, because of bizarre sqlite grammar
                };

                Constraint();
                Constraint(const Constraint& other);
                ~Constraint();
                SqliteStatement* clone();

                void initNameOnly(const QString& name);
                void initPk(const QList<SqliteIndexedColumn*>& indexedColumns,
                            bool autoincr, SqliteConflictAlgo algo);
                void initUnique(const QList<SqliteIndexedColumn*>& indexedColumns,
                                SqliteConflictAlgo algo);
                void initCheck(SqliteExpr* expr, SqliteConflictAlgo algo);
                void initCheck();
                void initFk(const QList<SqliteIndexedColumn*>& indexedColumns, const QString& table,
                            const QList<SqliteIndexedColumn*>& fkColumns, const QList<SqliteForeignKey::Condition*>& conditions,
                            SqliteInitially initially, SqliteDeferrable deferrable);

                bool doesAffectColumn(const QString& columnName);
                int getAffectedColumnIdx(const QString& columnName);
                QString typeString() const;

                Type type;
                QString name = QString();
                bool autoincrKw = false; // not in docs, but needs to be supported
                SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
                SqliteForeignKey* foreignKey = nullptr;
                SqliteExpr* expr = nullptr;
                QList<SqliteIndexedColumn*> indexedColumns;
                bool afterComma = false;

            protected:
                TokenList rebuildTokensFromContents();
        };

        typedef QSharedPointer<Constraint> ConstraintPtr;

        SqliteCreateTable();
        SqliteCreateTable(const SqliteCreateTable& other);
        SqliteCreateTable(bool ifNotExistsKw, int temp, const QString& name1, const QString& name2,
                          const QList<Column*>& columns, const QList<Constraint*>& constraints);
        SqliteCreateTable(bool ifNotExistsKw, int temp, const QString& name1, const QString& name2,
                          const QList<Column*>& columns, const QList<Constraint*>& constraints,
                          const QList<ParserStubCreateTableOption*>& options);
        SqliteCreateTable(bool ifNotExistsKw, int temp, const QString& name1, const QString& name2,
                          SqliteSelect* select);
        ~SqliteCreateTable();
        SqliteStatement* clone();

        QList<Constraint*> getConstraints(Constraint::Type type) const;
        SqliteStatement* getPrimaryKey() const;
        QStringList getPrimaryKeyColumns() const;
        Column* getColumn(const QString& colName);
        QList<Constraint*> getForeignKeysByTable(const QString& foreignTable) const;
        QList<Column::Constraint*> getColumnForeignKeysByTable(const QString& foreignTable) const;
        void removeColumnConstraint(Column::Constraint* constr);
        QStringList getColumnNames() const;
        QHash<QString,QString> getModifiedColumnsMap(bool lowercaseKeys = false, Qt::CaseSensitivity cs = Qt::CaseInsensitive) const;
        QString getTargetDatabase() const;
        void setTargetDatabase(const QString& database);
        QString getObjectName() const;
        void setObjectName(const QString& name);

        bool ifNotExistsKw = false;
        bool tempKw = false;
        bool temporaryKw = false;
        QString database = QString();
        QString table = QString();
        QList<Column*> columns;
        QList<Constraint*> constraints;
        bool withOutRowId = false;
        bool strict = false;
        SqliteSelect* select = nullptr;

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();

    private:
        void init(bool ifNotExistsKw, int temp, const QString& name1, const QString& name2);
};

typedef QSharedPointer<SqliteCreateTable> SqliteCreateTablePtr;

#endif // SQLITECREATETABLE_H
