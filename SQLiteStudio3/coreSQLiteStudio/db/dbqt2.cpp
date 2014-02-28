#include "dbqt2.h"
#include "parser/lexer.h"
#include <QDebug>

DbQt2::DbQt2(const QString& driverName, const QString& type) :
    DbQt(driverName, type)
{
}

SqlResultsPtr DbQt2::execInternal(const QString& query, const QList<QVariant>& args)
{
    // Sqlite2 seems to be dealing with only the '?' parameter placeholders. We need to update all of them.
    Lexer lexer(Dialect::Sqlite2);
    TokenList tokens = lexer.tokenize(query);
    TokenList filteredTokens = tokens.filter(Token::BIND_PARAM);
    foreach (TokenPtr token, filteredTokens)
        token->value = "?";

    QString newQuery = tokens.detokenize();
    return DbQt::execInternal(newQuery, args);
}

SqlResultsPtr DbQt2::execInternal(const QString& query, const QHash<QString, QVariant>& args)
{
    QList<QVariant> newArgs;

    Lexer lexer(Dialect::Sqlite2);
    TokenList tokens = lexer.tokenize(query);
    TokenList filteredTokens = tokens.filter(Token::BIND_PARAM);
    QString key;
    foreach (TokenPtr token, filteredTokens)
    {
        if (token->value.length() > 1)
        {
            key = token->value.mid(1);
            if (!args.contains(key))
            {
                qCritical() << "Named bind paramter cannot be found in hash (DbSqlite2Instance::execInternal()).";
                newArgs << QVariant();
            }
            else
                newArgs << args[key];
        }
        else
        {
            qCritical() << "Unnamed bind paramter but execInternal() called with hash (DbSqlite2Instance::execInternal()).";
            newArgs << QVariant();
        }

        token->value = "?";
    }

    QString newQuery = tokens.detokenize();
    return DbQt::execInternal(newQuery, newArgs);
}
