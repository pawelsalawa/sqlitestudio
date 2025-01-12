#ifndef SQLITEFILTEROVER_H
#define SQLITEFILTEROVER_H

#include "sqlitestatement.h"
#include "sqlitewindowdefinition.h"

class SqliteFilterOver : public SqliteStatement
{
    Q_OBJECT

    public:
        class Over : public SqliteStatement
        {
            public:
                enum class Mode
                {
                    WINDOW,
                    NAME
                };

                Over();
                Over(const Over& other);
                ~Over();
                Over(SqliteWindowDefinition::Window* window);
                Over(const QString& name);

                SqliteStatement* clone();

                SqliteWindowDefinition::Window* window = nullptr;
                QString name = QString();
                Mode mode = Mode::WINDOW;

            protected:
                TokenList rebuildTokensFromContents();
        };

        class Filter : public SqliteStatement
        {
            public:
                Filter();
                Filter(const Filter& other);
                ~Filter();
                Filter(SqliteExpr* expr);

                SqliteStatement* clone();

                SqliteExpr* expr;

            protected:
                TokenList rebuildTokensFromContents();
        };

        SqliteFilterOver();
        ~SqliteFilterOver();
        SqliteFilterOver(const SqliteFilterOver& other);
        SqliteFilterOver(Filter* filter, Over* over);

        SqliteStatement* clone();

        Filter* filter = nullptr;
        Over* over = nullptr;

    protected:
        TokenList rebuildTokensFromContents();
};

#endif // SQLITEFILTEROVER_H
