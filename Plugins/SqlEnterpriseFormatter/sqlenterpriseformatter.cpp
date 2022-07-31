#include "sqlenterpriseformatter.h"
#include "formatstatement.h"
#include "common/unused.h"
#include "common/global.h"
#include <QDebug>
#include <parser/lexer.h>
#include <parser/parser.h>

SqlEnterpriseFormatter::SqlEnterpriseFormatter()
{
}

QString SqlEnterpriseFormatter::format(SqliteQueryPtr query)
{
    QList<Comment*> comments = collectComments(query->tokens);

    int wrapperIdx = cfg.SqlEnterpriseFormatter.Wrappers.get().indexOf(cfg.SqlEnterpriseFormatter.PrefferedWrapper.get());

    NameWrapper wrapper = getAllNameWrappers()[wrapperIdx];

    FormatStatement *formatStmt = FormatStatement::forQuery(query.data());
    if (!formatStmt)
        return query->detokenize();

    formatStmt->setSelectedWrapper(wrapper);
    formatStmt->setConfig(&cfg);
    QString formatted = formatStmt->format();
    delete formatStmt;

    QString formattedWithComments = applyComments(formatted, comments);
    for (Comment* c : comments)
        delete c;

    return formattedWithComments;
}

bool SqlEnterpriseFormatter::init()
{
    SQLS_INIT_RESOURCE(sqlenterpriseformatter);

    static_qstring(query1, "SELECT (2 + 4) AND (3 + 5), 4 NOT IN (SELECT t1.'some[_]name' + t2.[some'name2] FROM xyz t1 JOIN zxc t2 ON (t1.aaa = t2.aaa)) "
                           "FROM a, (SELECT id FROM table2);");
    static_qstring(query2, "INSERT INTO table1 (id, value1, value2) VALUES (1, (2 + 5), (SELECT id FROM table2));");
    static_qstring(query3, "CREATE TABLE tab (id INTEGER PRIMARY KEY, /*a primary key column*/ value1 VARCHAR(6), "
                           "value2 /*column with constraints*/ NUMBER(8,2) NOT NULL DEFAULT 1.0"
                           ");");
    static_qstring(query4, "CREATE UNIQUE INDEX IF NOT EXISTS dbName.idx1 ON [messages column] (id COLLATE x ASC, lang DESC, description);");

    Parser parser;

    for (const QString& q : {query1, query2, query3, query4})
    {
        if (!parser.parse(q))
        {
            qWarning() << "SqlEnterpriseFormatter preview query parsing error:" << parser.getErrorString();
            continue;
        }
        previewQueries << parser.getQueries().first();
    }

    updatePreview();

    return GenericPlugin::init();
}

void SqlEnterpriseFormatter::deinit()
{
    SQLS_CLEANUP_RESOURCE(sqlenterpriseformatter);
}

void SqlEnterpriseFormatter::updatePreview()
{
    QStringList output;
    for (SqliteQueryPtr& q : previewQueries)
        output << format(q);

    cfg.SqlEnterpriseFormatter.PreviewCode.set(output.join("\n\n"));
}

void SqlEnterpriseFormatter::configModified(CfgEntry* entry)
{
    if (entry == &cfg.SqlEnterpriseFormatter.PreviewCode)
        return;

    updatePreview();
}

QString Cfg::getNameWrapperStr(NameWrapper wrapper)
{
    return wrapObjName(QObject::tr("name", "example name wrapper"), wrapper);
}

QStringList Cfg::getNameWrapperStrings()
{
    QStringList strings;
    for (NameWrapper nw : getAllNameWrappers())
        strings << wrapObjName(QObject::tr("name", "example name wrapper"), nw);

    return strings;
}

CfgMain* SqlEnterpriseFormatter::getMainUiConfig()
{
    return &cfg;
}

QString SqlEnterpriseFormatter::getConfigUiForm() const
{
    return "SqlEnterpriseFormatter";
}

void SqlEnterpriseFormatter::configDialogOpen()
{
    connect(&cfg.SqlEnterpriseFormatter, SIGNAL(changed(CfgEntry*)), this, SLOT(configModified(CfgEntry*)));
}

void SqlEnterpriseFormatter::configDialogClosed()
{
    disconnect(&cfg.SqlEnterpriseFormatter, SIGNAL(changed(CfgEntry*)), this, SLOT(configModified(CfgEntry*)));
}

QList<SqlEnterpriseFormatter::Comment *> SqlEnterpriseFormatter::collectComments(const TokenList &tokens)
{
    QList<Comment*> results;

    QList<TokenList> tokensInLines = tokensByLines(tokens);
    Comment* prevCommentInThisLine = nullptr;
    Comment* cmt = nullptr;
    bool tokensBefore = false;
    int pos = 0;
    int line = 0;
    for (const TokenList& tokensInLine : tokensInLines)
    {
        tokensBefore = true;
        prevCommentInThisLine = nullptr;
        for (const TokenPtr& token : tokensInLine)
        {
            if (token->type == Token::Type::SPACE)
                continue;

            if (prevCommentInThisLine)
                prevCommentInThisLine->tokensAfter = true;

            if (token->type == Token::Type::COMMENT)
            {
                cmt = new Comment;
                cmt->tokensBefore = tokensBefore;
                cmt->position = pos;
                cmt->multiline = token->value.startsWith("/*");
                if (cmt->multiline)
                    cmt->contents = token->value.mid(2, token->value.length() - 4).trimmed();
                else
                    cmt->contents = token->value.mid(2).trimmed();

                results << cmt;
                prevCommentInThisLine = cmt;
                continue;
            }

            tokensBefore = true;
            pos++;
        }
        line++;
    }

    return results;
}

QList<TokenList> SqlEnterpriseFormatter::tokensByLines(const TokenList &tokens, bool includeSpaces)
{
    QList<TokenList> tokensInLines;
    TokenList tokensInLine;
    for (const TokenPtr& token : tokens)
    {
        if (includeSpaces || token->type != Token::Type::SPACE)
            tokensInLine << token;

        if (token->type == Token::Type::SPACE && token->value.contains('\n'))
        {
            tokensInLines << tokensInLine;
            tokensInLine.clear();
        }
    }
    if (tokensInLine.size() > 0)
        tokensInLines << tokensInLine;

    return tokensInLines;
}

TokenList SqlEnterpriseFormatter::adjustCommentsToEnd(const TokenList &inputTokens)
{
    QList<TokenList> tokensInLines = tokensByLines(inputTokens, true);
    TokenList newTokens;
    TokenList commentTokensForLine;
    TokenPtr newLineToken;
    for (const TokenList& tokensInLine : tokensInLines)
    {
        commentTokensForLine.clear();
        newLineToken.clear();
        for (const TokenPtr& token : tokensInLine)
        {
            if (token->type == Token::Type::COMMENT)
            {
                wrapComment(token, true);
                //token->value = " " + endLineCommentTpl.arg(token->value);
                commentTokensForLine << token;
            }
            else if (token->type == Token::Type::SPACE && token->value.contains("\n"))
                newLineToken = token;
            else
                newTokens << token;
        }

        newTokens += commentTokensForLine;
        if (newLineToken)
            newTokens << newLineToken;
    }
    return newTokens;
}

TokenList SqlEnterpriseFormatter::wrapOnlyComments(const TokenList &inputTokens)
{
    QList<TokenList> tokensInLines = tokensByLines(inputTokens, true);
    TokenList newTokens;
    bool lineEnd = true;
    for (const TokenList& tokensInLine : reverse(tokensInLines))
    {
        lineEnd = true;
        for (const TokenPtr& token : reverse(tokensInLine))
        {
            if (!token->isWhitespace())
                lineEnd = false;

            if (token->type == Token::Type::COMMENT)
                wrapComment(token, lineEnd);

            newTokens << token;
        }
    }
    return reverse(newTokens);
}

TokenList SqlEnterpriseFormatter::optimizeInnerComments(const TokenList &inputTokens)
{
    // TODO
    return inputTokens;
}

TokenList SqlEnterpriseFormatter::optimizeEndLineComments(const TokenList &inputTokens)
{
    // TODO
    return inputTokens;
}

void SqlEnterpriseFormatter::indentMultiLineComments(const TokenList &inputTokens)
{
    UNUSED(inputTokens);
    // TODO
}

void SqlEnterpriseFormatter::wrapComment(const TokenPtr &token, bool isAtLineEnd)
{
    static_qstring(multiCommentTpl, "/* %1 */");
    static_qstring(endLineCommentTpl, "-- %1");

    bool isMultiLine = token->value.contains("\n");
    if (isAtLineEnd && !isMultiLine && cfg.SqlEnterpriseFormatter.PreferredCommentMarker.get() == "--")
        token->value = endLineCommentTpl.arg(token->value);
    else
        token->value = multiCommentTpl.arg(token->value);
}

QString SqlEnterpriseFormatter::applyComments(const QString& formatted, QList<SqlEnterpriseFormatter::Comment*> comments)
{
    if (comments.size() == 0)
        return formatted;

    int currentCommentPosition = comments.first()->position;

    TokenList allTokens = Lexer::tokenize(formatted);
    TokenList newTokens;
    int currentTokenPosition = 0;
    for (const TokenPtr& token : allTokens)
    {
        if (currentTokenPosition == currentCommentPosition)
        {
            newTokens << TokenPtr::create(Token::Type::COMMENT, comments.first()->contents);
            comments.removeFirst();
            if (comments.size() > 0)
                currentCommentPosition = comments.first()->position;
            else
                currentCommentPosition = -1;
        }

        newTokens << token;
        if (token->type != Token::Type::SPACE)
            currentTokenPosition++;
    }

    // Any remaining comments
    for (Comment* cmt : comments)
        newTokens << TokenPtr::create(Token::Type::COMMENT, cmt->contents);

    if (cfg.SqlEnterpriseFormatter.MoveAllCommentsToLineEnd.get())
        newTokens = adjustCommentsToEnd(newTokens);
    else
        newTokens = wrapOnlyComments(newTokens);

    newTokens = optimizeInnerComments(newTokens);
    newTokens = optimizeEndLineComments(newTokens);
    indentMultiLineComments(newTokens);

    return newTokens.detokenize();
}
