#ifndef SQLITEWINDOWDEFINITION_H
#define SQLITEWINDOWDEFINITION_H

//#include "sqliteexpr.h"
//#include "sqliteorderby.h"
#include "sqlitestatement.h"

class SqliteExpr;
class SqliteOrderBy;

/**
 * @addtogroup sqlite_statement
 * @brief The SqliteWindowDefinition class
 */
class API_EXPORT SqliteWindowDefinition : public SqliteStatement
{
    Q_OBJECT

    public:
        class API_EXPORT Window : public SqliteStatement
        {
            public:
                class API_EXPORT Frame : public SqliteStatement
                {
                    public:
                        class API_EXPORT Bound : public SqliteStatement
                        {
                            public:
                                enum class Type
                                {
                                    UNBOUNDED_PRECEDING,
                                    UNBOUNDED_FOLLOWING,
                                    EXPR_PRECEDING,
                                    EXPR_FOLLOWING,
                                    CURRENT_ROW
                                };

                                Bound();
                                Bound(const Bound& other);
                                Bound(SqliteExpr* expr, const QString& value);

                                SqliteStatement* clone();

                                Type type = Type::CURRENT_ROW;
                                SqliteExpr* expr = nullptr;

                            protected:
                                TokenList rebuildTokensFromContents();
                        };

                        enum class RangeOrRows
                        {
                            RANGE,
                            ROWS,
                            GROUPS,
                            null
                        };

                        enum class Exclude
                        {
                            NO_OTHERS,
                            CURRENT_ROW,
                            GROUP,
                            TIES,
                            null
                        };

                        static RangeOrRows toRangeOrRows(const QString& value);
                        static QString fromRangeOrRows(RangeOrRows value);
                        static Exclude toExclude(const QString& value);
                        static QString fromExclude(Exclude value);

                        Frame();
                        Frame(const Frame& other);
                        Frame(RangeOrRows rangeOrRows, Bound* startBound, Bound* endBound, Exclude exclude);

                        SqliteStatement* clone();

                        RangeOrRows rangeOrRows = RangeOrRows::null;
                        Exclude exclude = Exclude::null;
                        Bound* startBound = nullptr;
                        Bound* endBound = nullptr;

                    protected:
                        TokenList rebuildTokensFromContents();
                };

                enum class Mode
                {
                    PARTITION_BY,
                    ORDER_BY,
                    null
                };

                Window();
                Window(const Window& other);

                SqliteStatement* clone();
                void initPartitionBy(const QString& name, const QList<SqliteExpr*>& exprList, const QList<SqliteOrderBy*>& orderBy, Frame* frame);
                void initOrderBy(const QString& name, const QList<SqliteOrderBy*>& orderBy, Frame* frame);
                void init(const QString& name, Frame* frame);

                QString name;
                QList<SqliteExpr*> exprList;
                QList<SqliteOrderBy*> orderBy;
                Frame* frame = nullptr;
                Mode mode = Mode::null;

            protected:
                TokenList rebuildTokensFromContents();

            private:
                void initExprList(const QList<SqliteExpr*>& exprList);
                void initOrderBy(const QList<SqliteOrderBy*>& orderBy);
                void initFrame(Frame* frame);
        };

        SqliteWindowDefinition();
        SqliteWindowDefinition(const SqliteWindowDefinition& other);
        SqliteWindowDefinition(const QString& name, Window* window);

        SqliteStatement* clone();

        QString name = QString();
        Window* window = nullptr;

    protected:
        TokenList rebuildTokensFromContents();
};

#endif // SQLITEWINDOWDEFINITION_H
