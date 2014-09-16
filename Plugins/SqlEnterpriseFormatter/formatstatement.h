#ifndef FORMATSTATEMENT_H
#define FORMATSTATEMENT_H

#include "parser/ast/sqlitestatement.h"
#include "common/utils_sql.h"
#include <QString>
#include <QStringList>
#include <QHash>
#include <QStack>
#include <QVariant>

class FormatStatement
{
    public:
        FormatStatement();
        virtual ~FormatStatement();

        QString format();
        void setSelectedWrapper(NameWrapper wrapper);

        static FormatStatement* forQuery(SqliteStatement *query);

        enum class ListSeparator
        {
            NONE,
            COMMA
        };

        virtual void formatInternal() = 0;
        virtual void resetInternal();
        void keywordToLineUp(const QString& keyword);
        FormatStatement& withKeyword(const QString& kw);
        FormatStatement& withLinedUpKeyword(const QString& kw);
        FormatStatement& withId(const QString& id);
        FormatStatement& withIdList(const QStringList& names, const QString& indentName = QString(), ListSeparator sep = ListSeparator::COMMA);
        FormatStatement& withOperator(const QString& oper);
        FormatStatement& withIdDot();
        FormatStatement& withStar();
        FormatStatement& withFloat(double value);
        FormatStatement& withInteger(qint64 value);
        FormatStatement& withString(const QString& value);
        FormatStatement& withBlob(const QString& value);
        FormatStatement& withBindParam(const QString& name);
        FormatStatement& withParDefLeft();
        FormatStatement& withParDefRight();
        FormatStatement& withParExprLeft();
        FormatStatement& withParExprRight();
        FormatStatement& withParFuncLeft();
        FormatStatement& withParFuncRight();
        FormatStatement& withSemicolon();
        FormatStatement& withListComma();
        FormatStatement& withCommaOper();
        FormatStatement& withFuncId(const QString& func);
        FormatStatement& withDataType(const QString& dataType);
        FormatStatement& withNewLine();
        FormatStatement& withLiteral(const QVariant& value);
        FormatStatement& withStatement(const QString& contents, const QString& indentName = QString());
        FormatStatement& withStatement(SqliteStatement* stmt, const QString& indentName = QString());
        FormatStatement& withLinedUpStatement(int prefixLength, const QString& contents, const QString& indentName = QString());
        FormatStatement& withLinedUpStatement(int prefixLength, SqliteStatement* stmt, const QString& indentName = QString());
        FormatStatement& markIndentForNextToken(const QString& name);
        FormatStatement& markIndentForLastToken(const QString& name);
        FormatStatement& markAndKeepIndent(const QString& name);
        FormatStatement& incrIndent(const QString& name = QString());
        FormatStatement& decrIndent();

        template <class T>
        FormatStatement& withStatementList(QList<T*> stmtList, const QString& indentName = QString(), ListSeparator sep = ListSeparator::COMMA)
        {
            if (!indentName.isNull())
                markAndKeepIndent(indentName);

            bool first = true;
            foreach (T* stmt, stmtList)
            {
                if (!first)
                {
                    switch (sep)
                    {
                        case ListSeparator::COMMA:
                            withListComma();
                            break;
                        case ListSeparator::NONE:
                            break;
                    }
                }

                withStatement(stmt);
                first = false;
            }

            if (!indentName.isNull())
                decrIndent();

            return *this;
        }

    protected:
        Dialect dialect = Dialect::Sqlite3;

    private:
        struct FormatToken
        {
            enum Type
            {
                KEYWORD,
                LINED_UP_KEYWORD,
                ID,
                OPERATOR,
                STAR,
                FLOAT,
                STRING,
                INTEGER,
                BLOB,
                BIND_PARAM,
                ID_DOT,
                PAR_DEF_LEFT,
                PAR_DEF_RIGHT,
                PAR_EXPR_LEFT,
                PAR_EXPR_RIGHT,
                PAR_FUNC_LEFT,
                PAR_FUNC_RIGHT,
                SEMICOLON,
                COMMA_LIST,
                COMMA_OPER, // for example in LIMIT
                FUNC_ID,
                DATA_TYPE,
                NEW_LINE,
                NEW_LINE_WITH_STATEMENT,
                STATEMENT,
                LINED_UP_STATEMENT,
                INDENT_MARKER,
                INCR_INDENT,
                DECR_INDENT
            };

            Type type;
            QVariant value;
            int lineUpPrefixLength = 0;
            QString indentMarkName;
        };

        void withToken(FormatToken::Type type, const QVariant& value);
        void withToken(FormatToken::Type type, const QVariant& value, int lineUpPrefixLength);
        void cleanup();
        QString detokenize();

        static FormatStatement* forQuery(SqliteStatement *query, Dialect dialect, NameWrapper wrapper);

        int kwLineUpPosition = 0;
        NameWrapper wrapper = NameWrapper::BRACKET;
        QHash<QString,int> namedIndents;
        QStack<int> indents;
        QList<FormatToken*> tokens;

        static const QString SPACE;
};

#endif // FORMATSTATEMENT_H
