#ifndef FORMATCREATETABLE_H
#define FORMATCREATETABLE_H

#include "formatstatement.h"
#include "parser/ast/sqlitecreatetable.h"

class FormatCreateTable : public FormatStatement
{
    public:
        FormatCreateTable(SqliteCreateTable* createTable);

    protected:
        void formatInternal();

    private:
        void formatColumns(const QList<SqliteCreateTable::Column*>& columns);
        int getColNameLength(const QString& name);

        SqliteCreateTable* createTable = nullptr;
};

class FormatCreateTableColumn : public FormatStatement
{
    public:
        FormatCreateTableColumn(SqliteCreateTable::Column* column);

        void setColNameIndent(int value);
        void setColTypeIndent(int value);

    protected:
        void formatInternal();

    private:
        SqliteCreateTable::Column* column = nullptr;
        int colNameIndent = 0;
        int colTypeIndent = 0;
};

class FormatCreateTableColumnConstraint : public FormatStatement
{
    public:
        FormatCreateTableColumnConstraint(SqliteCreateTable::Column::Constraint* constr);

    protected:
        void formatInternal();

    private:
        SqliteCreateTable::Column::Constraint* constr = nullptr;
};

class FormatCreateTableConstraint : public FormatStatement
{
    public:
        FormatCreateTableConstraint(SqliteCreateTable::Constraint* constr);

    protected:
        void formatInternal();

    private:
        SqliteCreateTable::Constraint* constr = nullptr;
};

#endif // FORMATCREATETABLE_H
