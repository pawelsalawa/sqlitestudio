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
            COMMA,
            EXPR_COMMA
        };

        FormatStatement& withKeyword(const QString& kw);
        FormatStatement& withLinedUpKeyword(const QString& kw, const QString& lineUpName = QString());
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
        FormatStatement& withStatement(SqliteStatement* stmt, const QString& indentName = QString());
        FormatStatement& markIndent(const QString& name);
        FormatStatement& markAndKeepIndent(const QString& name);
        FormatStatement& withIncrIndent(const QString& name = QString());
        FormatStatement& withDecrIndent();
        FormatStatement& markKeywordLineUp(const QString& keyword, const QString& lineUpName = QString());

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
                        case ListSeparator::EXPR_COMMA:
                            withCommaOper();
                            break;
                        case ListSeparator::NONE:
                            break;
                    }
                }

                withStatement(stmt);
                first = false;
            }

            if (!indentName.isNull())
                withDecrIndent();

            return *this;
        }

    protected:
        virtual void formatInternal() = 0;
        virtual void resetInternal();

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
                INDENT_MARKER,
                INCR_INDENT,
                DECR_INDENT,
                MARK_KEYWORD_LINEUP
            };

            Type type;
            QVariant value;
            QVariant additionalValue;
        };

        void withToken(FormatToken::Type type, const QVariant& value, const QVariant& additionalValue = QVariant());
        void cleanup();
        void buildTokens();
        QString detokenize();
        bool applyIndent();
        void applySpace(FormatToken::Type type);
        bool isSpaceExpectingType(FormatToken::Type type);
        bool isMetaType(FormatToken::Type type);
        void newLine();
        void incrIndent(const QString& name = QString());
        void decrIndent();
        bool endsWithSpace();
        void detokenizeLeftPar(FormatToken* token, bool spaceBefore, bool spaceAfter, bool nlBefore, bool nlAfter);
        void detokenizeRightPar(FormatToken* token, bool spaceBefore, bool spaceAfter, bool nlBefore, bool nlAfter);
        void resetIndents();
        void updateLastToken(FormatToken* token);
        QString getFinalLineUpName(const QString& lineUpName);
        int predictCurrentIndent(FormatToken* currentMetaToken);
        bool willStartWithNewLine(FormatToken* token);

        static FormatStatement* forQuery(SqliteStatement *query, Dialect dialect, NameWrapper wrapper);

        QHash<QString,int> kwLineUpPosition;
        NameWrapper wrapper = NameWrapper::BRACKET;
        QHash<QString,int> namedIndents;
        QStack<int> indents;
        QList<FormatToken*> tokens;
        bool deleteTokens = true;
        QStringList lines;
        QString line;
        FormatToken* lastToken = nullptr;
        QString statementName;

        static qint64 nameSeq;
        static const QString SPACE;
        static const QString NEWLINE;
};

#endif // FORMATSTATEMENT_H
