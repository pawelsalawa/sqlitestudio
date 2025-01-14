#ifndef SQLITEEXPR_H
#define SQLITEEXPR_H

#include "sqlitestatement.h"
#include <QString>
#include <QVariant>
#include <QList>

class SqliteFilterOver;
class SqliteSelect;
class SqliteColumnType;
class SqliteRaise;

class API_EXPORT SqliteExpr : public SqliteStatement
{
    Q_OBJECT

    public:
        enum class Mode
        {
            null,
            LITERAL_VALUE,
            CTIME,
            BIND_PARAM,
            ID,
            UNARY_OP,
            BINARY_OP,
            PTR_OP,
            FUNCTION,
            SUB_EXPR,
            ROW_VALUE,
            CAST,
            COLLATE,
            LIKE,
            NULL_,
            NOTNULL,
            IS,
            DISTINCT,
            BETWEEN,
            IN,
            EXISTS,
            CASE,
            SUB_SELECT,
            RAISE,
            WINDOW_FUNCTION
        };

        enum class NotNull
        {
            ISNULL,
            NOT_NULL,
            NOTNULL,
            null
        };

        enum class LikeOp
        {
            LIKE,
            GLOB,
            REGEXP,
            MATCH,
            null
        };

        SqliteExpr();
        SqliteExpr(const SqliteExpr& other);
        ~SqliteExpr();

        static LikeOp likeOp(const QString& value);
        static QString likeOp(LikeOp value);
        static NotNull notNullOp(const QString& value);
        static QString notNullOp(NotNull value);

        SqliteStatement* clone();
        void initLiteral(const QVariant& value);
        void initNull();
        void initCTime(const QString& name);
        void initSubExpr(SqliteExpr* expr);
        void initRowValue(const QList<SqliteExpr*>& exprList);
        void initId(const QString& db, const QString& table, const QString& column);
        void initId(const QString& table, const QString& column);
        void initId(const QString& column);
        void initBindParam(const QString& value);
        void initCollate(SqliteExpr* expr, const QString& value);
        void initCast(SqliteExpr* expr, SqliteColumnType* type);
        void initFunction(const QString& fnName, int distinct, const QList<SqliteExpr*>& exprList);
        void initFunction(const QString& fnName, bool star = false);
        void initWindowFunction(const QString& fnName, int distinct, const QList<SqliteExpr*>& exprList, SqliteFilterOver* filterOver);
        void initWindowFunction(const QString& fnName, SqliteFilterOver* filterOver);
        void initBinOp(SqliteExpr* expr1, const QString& op, SqliteExpr* expr2);
        void initUnaryOp(SqliteExpr* expr, const QString& op);
        void initPtrOp(SqliteExpr* expr1, const QString& op, SqliteExpr* expr2);
        void initLike(SqliteExpr* expr1, bool notKw, SqliteExpr::LikeOp likeOp, SqliteExpr* expr2, SqliteExpr* expr3 = nullptr);
        void initNull(SqliteExpr* expr, const QString& value);
        void initIs(SqliteExpr* expr1, bool notKw, SqliteExpr* expr2);
        void initDistinct(SqliteExpr* expr1, bool notKw, SqliteExpr* expr2);
        void initBetween(SqliteExpr* expr1, bool notKw, SqliteExpr* expr2, SqliteExpr* expr3);
        void initIn(SqliteExpr* expr, bool notKw, const QList<SqliteExpr*>& exprList);
        void initIn(SqliteExpr* expr, bool notKw, SqliteSelect* select);
        void initIn(SqliteExpr* expr, bool notKw, const QString& name1, const QString& name2);
        void initExists(SqliteSelect* select);
        void initSubSelect(SqliteSelect* select);
        void initCase(SqliteExpr* expr1, const QList<SqliteExpr*>& exprList, SqliteExpr* expr2);
        void initRaise(const QString& type, const QString& text = QString());
        void detectDoubleQuotes(bool recursively = true);
        bool replace(SqliteExpr* toBeReplaced, SqliteExpr* replaceWith);

        Mode mode = Mode::null;
        QVariant literalValue = QVariant();
        bool literalNull = false;
        QString bindParam = QString();
        QString database = QString();
        QString table = QString();
        QString column = QString();
        QString unaryOp = QString();
        QString binaryOp = QString();
        QString ptrOp = QString();
        QString function = QString();
        QString collation = QString();
        QString ctime = QString();
        SqliteColumnType* columnType = nullptr;
        SqliteExpr* expr1 = nullptr;
        SqliteExpr* expr2 = nullptr;
        SqliteExpr* expr3 = nullptr;
        QList<SqliteExpr*> exprList;
        SqliteSelect* select = nullptr;
        SqliteFilterOver* filterOver = nullptr;
        bool distinctKw = false;
        bool allKw = false; // alias for DISTINCT as for sqlite3 grammar
        bool star = false;
        bool notKw = false;
        LikeOp like = LikeOp::null;
        NotNull notNull = NotNull::null;
        SqliteRaise* raiseFunction = nullptr;
        bool possibleDoubleQuotedString = false;

    protected:
        QStringList getColumnsInStatement();
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getColumnTokensInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
        void evaluatePostParsing();

    private:
        bool doubleQuotesChecked = false;
        TokenList rebuildId();
        TokenList rebuildLike();
        TokenList rebuildNotNull();
        TokenList rebuildIs();
        TokenList rebuildDistinct();
        TokenList rebuildBetween();
        TokenList rebuildIn();
        TokenList rebuildCase();
        void initDistinct(int distinct);
};

typedef QSharedPointer<SqliteExpr> SqliteExprPtr;

#endif // SQLITEEXPR_H
