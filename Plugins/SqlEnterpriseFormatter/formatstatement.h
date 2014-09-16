#ifndef FORMATSTATEMENT_H
#define FORMATSTATEMENT_H

#include "parser/ast/sqlitestatement.h"
#include "common/utils_sql.h"
#include <QString>
#include <QStringList>
#include <QStack>

class FormatStatement
{
    public:
        FormatStatement();
        virtual ~FormatStatement();

        QString format();
        void setSelectedWrapper(NameWrapper wrapper);

        static FormatStatement* forQuery(SqliteStatement *query);

    protected:
        virtual void formatInternal() = 0;
        virtual void resetInternal() = 0;
        void keywordToLineUp(const QString& keyword);
        void pushIndent();
        void popIndent();
        void newLine();
        void append(const QString& str);
        void appendKeyword(const QString& str);
        void appendDataType(const QString& str);
        void appendLeftExprPar();
        void appendRightExprPar();
        void appendLeftDefPar();
        void appendRightDefPar();
        void appendName(const QString& str);
        void appendLinedUpKeyword(const QString& str);
        void appendLinedUpPrefixedString(const QString& prefixWord, const QString& str);
        void appendNameDot();
        QString format(SqliteStatement* stmt);
        QString formatExprList(const QList<SqliteStatement*>& stmts);
        QString formatDefList(const QList<SqliteStatement*>& stmts);

    private:
        QString formatStmtList(const QList<SqliteStatement*>& stmts, bool spaceBeforeSep, bool spaceAfterSep, bool nlAfterSep);
        void appendPar(const QString &par, bool nlBefore, bool spaceBefore, bool nlAfter, bool spaceAfter);
        void trimLineEnd();

        int kwLineUpPosition = 0;
        QStringList lines;
        QString line;
        QStack<int> indents;
        Dialect dialect = Dialect::Sqlite3;
        NameWrapper wrapper = NameWrapper::BRACKET;

        static const QString SPACE;
};

#endif // FORMATSTATEMENT_H
