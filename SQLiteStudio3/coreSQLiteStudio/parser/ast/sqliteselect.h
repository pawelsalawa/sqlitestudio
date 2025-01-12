#ifndef SQLITESELECT_H
#define SQLITESELECT_H

#include "sqlitequery.h"
#include "sqliteexpr.h"
#include "sqlitelimit.h"
#include "sqliteorderby.h"

#include <QList>

class SqliteWith;
class SqliteWindowDefinition;

/**
 * @addtogroup sqlite_statement
 * @brief The SqliteSelect class
 */
class API_EXPORT SqliteSelect : public SqliteQuery
{
    Q_OBJECT

    public:
        enum class CompoundOperator
        {
            UNION,
            UNION_ALL,
            INTERSECT,
            EXCEPT,
            null
        };

        class API_EXPORT Core : public SqliteStatement
        {
            public:
                class API_EXPORT ResultColumn : public SqliteStatement
                {
                    public:
                        ResultColumn();
                        ResultColumn(const ResultColumn& other);
                        ResultColumn(SqliteExpr* expr, bool asKw, const QString& alias);
                        ResultColumn(bool star, const QString& table);
                        explicit ResultColumn(bool star);

                        bool isRowId();
                        SqliteStatement* clone();

                        SqliteExpr* expr = nullptr;
                        bool star = false;
                        bool asKw = false;
                        QString alias = QString();
                        QString table = QString();

                    protected:
                        QStringList getTablesInStatement();
                        TokenList getTableTokensInStatement();
                        QList<FullObject> getFullObjectsInStatement();
                        TokenList rebuildTokensFromContents();
                };

                class JoinSource; // forward declaration

                class API_EXPORT SingleSource : public SqliteStatement
                {
                    public:
                        SingleSource();
                        SingleSource(const SingleSource& other);
                        SingleSource(const QString& name1, const QString& name2,
                                     bool asKw, const QString& alias, bool notIndexedKw, const QString& indexedBy);
                        SingleSource(const QString& name1, const QString& name2,
                                     bool asKw, const QString& alias, const QList<SqliteExpr*>& exprList);
                        SingleSource(SqliteSelect* select, bool asKw, const QString& alias);
                        SingleSource(JoinSource* src, bool asKw, const QString& alias);

                        SqliteStatement* clone();

                        QString database = QString();
                        QString table = QString();
                        QString alias = QString();
                        QString funcName = QString();
                        QList<SqliteExpr*> funcParams;
                        bool asKw = false;
                        bool indexedByKw = false;
                        bool notIndexedKw = false;
                        QString indexedBy = QString();
                        SqliteSelect* select = nullptr;
                        JoinSource* joinSource = nullptr;

                    protected:
                        QStringList getTablesInStatement();
                        QStringList getDatabasesInStatement();
                        TokenList getTableTokensInStatement();
                        TokenList getDatabaseTokensInStatement();
                        QList<FullObject> getFullObjectsInStatement();
                        TokenList rebuildTokensFromContents();
                };

                class API_EXPORT JoinOp : public SqliteStatement
                {
                    public:
                        JoinOp();
                        JoinOp(const JoinOp& other);
                        explicit JoinOp(bool comma);
                        explicit JoinOp(const QString& joinToken);
                        JoinOp(const QString& joinToken, const QString& word1);
                        JoinOp(const QString& joinToken, const QString& word1, const QString& word2);

                        SqliteStatement* clone();

                    private:
                        void init(const QString& str);

                    public:
                        bool comma = false;
                        bool joinKw = false;
                        bool naturalKw = false;
                        bool leftKw = false;
                        bool outerKw = false;
                        bool innerKw = false;
                        bool crossKw = false;
                        bool rightKw = false;
                        bool fullKw = false;
                        QString customKw1 = QString();
                        QString customKw2 = QString();
                        QString customKw3 = QString();

                    protected:
                        TokenList rebuildTokensFromContents();
                };

                class API_EXPORT JoinConstraint : public SqliteStatement
                {
                    public:
                        JoinConstraint();
                        JoinConstraint(const JoinConstraint& other);
                        explicit JoinConstraint(SqliteExpr* expr);
                        explicit JoinConstraint(const QList<QString>& strList);

                        SqliteStatement* clone();

                        SqliteExpr* expr = nullptr;
                        QList<QString> columnNames;

                    protected:
                        QStringList getColumnsInStatement();
                        TokenList getColumnTokensInStatement();
                        TokenList rebuildTokensFromContents();
                };

                class API_EXPORT JoinSourceOther : public SqliteStatement
                {
                    public:
                        JoinSourceOther();
                        JoinSourceOther(const JoinSourceOther& other);
                        JoinSourceOther(SqliteSelect::Core::JoinOp *op,
                                        SqliteSelect::Core::SingleSource* src,
                                        SqliteSelect::Core::JoinConstraint* constr);

                        SqliteStatement* clone();

                        JoinOp* joinOp = nullptr;
                        SingleSource* singleSource = nullptr;
                        JoinConstraint* joinConstraint = nullptr;

                    protected:
                        TokenList rebuildTokensFromContents();
                };

                class API_EXPORT JoinSource : public SqliteStatement
                {
                    public:
                        JoinSource();
                        JoinSource(const JoinSource& other);
                        JoinSource(SingleSource* singleSource, const QList<JoinSourceOther*>& list);

                        SqliteStatement* clone();

                        SingleSource* singleSource = nullptr;
                        QList<JoinSourceOther*> otherSources;

                    protected:
                        TokenList rebuildTokensFromContents();
                };

                Core();
                Core(const Core& other);
                Core(int distinct, const QList<ResultColumn*>& resCols, JoinSource* src, SqliteExpr* where,
                     const QList<SqliteExpr*>& groupBy, SqliteExpr* having,
                     const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit);
                Core(int distinct, const QList<ResultColumn*>& resCols, JoinSource* src, SqliteExpr* where,
                     const QList<SqliteExpr*>& groupBy, SqliteExpr* having, const QList<SqliteWindowDefinition*> windows,
                     const QList<SqliteOrderBy*>& orderBy, SqliteLimit* limit);

                SqliteStatement* clone();

                CompoundOperator compoundOp = CompoundOperator::null;
                QList<ResultColumn*> resultColumns;
                JoinSource* from = nullptr;
                bool distinctKw = false;
                bool allKw = false;
                SqliteExpr* where = nullptr;
                SqliteExpr* having = nullptr;
                QList<SqliteExpr*> groupBy;
                QList<SqliteOrderBy*> orderBy;
                QList<SqliteWindowDefinition*> windows;
                SqliteLimit* limit = nullptr;
                bool valuesMode = false;

            protected:
                TokenList rebuildTokensFromContents();
        };

        SqliteSelect();
        SqliteSelect(const SqliteSelect& other);

        static SqliteSelect* append(SqliteSelect::Core* core);
        static SqliteSelect* append(SqliteSelect* select, CompoundOperator op, SqliteSelect::Core* core);
        static SqliteSelect* append(const QList<QList<SqliteExpr*>>& values);
        static SqliteSelect* append(SqliteSelect* select, SqliteSelect::CompoundOperator op, const QList<QList<SqliteExpr*>>& values);

        SqliteStatement* clone();
        QString compoundOperator(CompoundOperator op);
        CompoundOperator compoundOperator(const QString& op);
        void reset();
        void setWith(SqliteWith* with);

        QList<Core*> coreSelects;
        SqliteWith* with = nullptr;

    protected:
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteSelect> SqliteSelectPtr;
typedef SqliteSelect::Core::ResultColumn SqliteResultColumn;

#endif // SQLITESELECT_H
