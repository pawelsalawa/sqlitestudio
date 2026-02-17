#include "sqlenterpriseformatter.h"
#include "formatstatement.h"
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

    NameWrapper wrapper = getAllNameWrappers().value(wrapperIdx);

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

QList<SqlEnterpriseFormatter::Comment*> SqlEnterpriseFormatter::collectComments(const TokenList &tokens)
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
        tokensBefore = false;
        prevCommentInThisLine = nullptr;
        for (const TokenPtr& token : tokensInLine)
        {
            if (token->type == Token::SPACE)
                continue;

            if (prevCommentInThisLine)
                prevCommentInThisLine->tokensAfter = true;

            if (token->type == Token::COMMENT)
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
        if (includeSpaces || token->type != Token::SPACE)
            tokensInLine << token;

        if (token->type == Token::SPACE && token->value.contains('\n'))
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
    for (const TokenList& tokensInLine : tokensInLines)
    {
        TokenList commentTokensForLine;
        TokenList regularTokensForLine;
        TokenPtr newLineToken;
        for (const TokenPtr& token : tokensInLine)
        {
            if (token->type == Token::COMMENT)
            {
                wrapComment(token, true);
                commentTokensForLine << token;
            }
            else if (token->type == Token::SPACE && token->value.contains("\n"))
                newLineToken = token;
            else
            {
                newTokens << token;
                regularTokensForLine << token;
            }
        }

        if (!regularTokensForLine.isEmpty() && regularTokensForLine.last()->type != Token::SPACE && !commentTokensForLine.isEmpty())
            newTokens << TokenPtr::create(Token::SPACE, " ");

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

            if (token->type == Token::COMMENT)
                wrapComment(token, lineEnd);

            newTokens << token;
        }
    }
    return reverse(newTokens);
}

void SqlEnterpriseFormatter::formatEndLineComments(TokenList& inputTokens, QList<TokenList>& lines)
{
    bool lineUp = cfg.SqlEnterpriseFormatter.LineUpCommentsAtLineEnd.get();
    if (lineUp)
        formatEndLineCommentsLinedUp(inputTokens, lines);
    else
        formatEndLineCommentsStandard(inputTokens, lines);
}

void SqlEnterpriseFormatter::formatEndLineCommentsStandard(TokenList& inputTokens, QList<TokenList>& lines)
{
    TokenList prevLine;
    for (TokenList& line : lines)
    {
        TokenPtr token = line | FIND_FIRST(token, {return token->type != Token::SPACE;});
        if (token && token->type != Token::COMMENT)
        {
            prevLine = line;
            continue;
        }

        TokenPtr prevLineToken = prevLine | FIND_FIRST(token, {return token->type != Token::SPACE;});
        int prevIndent = prevLineToken ? prevLineToken->start : 0;

        if (token->start < prevIndent)
        {
            int indentDiff = prevIndent - token->start;
            TokenPtr indentToken = TokenPtr::create(Token::SPACE, QString(indentDiff, ' '));
            line.insert(0, indentToken);
            inputTokens.insert(inputTokens.indexOf(token), indentToken);
            line.reindexPositions();
        }

        prevLine = line;
    }
}

void SqlEnterpriseFormatter::formatEndLineCommentsLinedUp(TokenList& inputTokens, QList<TokenList>& lines)
{
    bool skippingInitialLines = true;
    int beginningLineSkipStopAt = 0;
    int skipEndLines = 0;
    int maxIndent = 0;
    for (const TokenList& line : lines)
    {
        bool hasMeaningful = line.hasMeaningfulToken();
        if (hasMeaningful)
        {
            skippingInitialLines = false;
            skipEndLines = 0;
        }
        else
            skipEndLines++;

        if (skippingInitialLines)
            beginningLineSkipStopAt++;

        TokenPtr token = line | FIND_LAST(token, {return token->type == Token::COMMENT;});
        if (!token || token->type != Token::COMMENT)
            continue;

        bool commentIsNotLast = false;
        for (int i = line.indexOf(token) + 1, total = line.size(); i < total; i++)
        {
            if (!line[i]->isWhitespace())
            {
                commentIsNotLast = true;
                break;
            }
        }
        if (commentIsNotLast)
            continue;

        maxIndent = qMax(maxIndent, token->start);
    }

    if (maxIndent == 0)
        return;

    int endingLineSkipStartAt = lines.size() - skipEndLines;
    int lineIdx = -1;
    for (TokenList& line : lines)
    {
        lineIdx++;
        if (lineIdx < beginningLineSkipStopAt)
            continue;

        if (lineIdx >= endingLineSkipStartAt)
            break;

        TokenPtr token = line | FIND_LAST(token, {return token->type == Token::COMMENT;});
        if (!token)
            continue;

        int tokenLineIdx = line.indexOf(token);
        if (line.mid(tokenLineIdx + 1).hasMeaningfulToken())
            continue;

        if (token->start < maxIndent)
        {
            int indentDiff = maxIndent - token->start;
            TokenPtr indentToken = TokenPtr::create(Token::SPACE, QString(indentDiff, ' '));
            line.insert(tokenLineIdx, indentToken);
            inputTokens.insert(inputTokens.indexOf(token), indentToken);
            line.reindexPositions();
        }
    }
}

void SqlEnterpriseFormatter::indentMultiLineComments(const TokenList& inputTokens, const QList<TokenList>& lines)
{
    Q_UNUSED(inputTokens);

    for (const TokenList& line : lines)
    {
        // Tokens already are indexed according to their lines in formatEndLineComments(), so we can just check the first token for comment indenting
        for (TokenPtr& token : line.filter(Token::COMMENT))
        {
            if (!token->value.startsWith("/*"))
                continue;

            if (!token->value.contains("\n"))
                continue;

            int indent = token->start + 3; // +3 for the beginning "/* " in 1st line of comment
            QStringList parts = token->value.split("\n");
            for (int i = 1; i < parts.size(); i++)
                parts[i] = QString(indent, ' ') + parts[i].trimmed();

            token->value = parts.join("\n");
        }
    }
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
    Lexer::setupPrevNextLinks(allTokens);
    TokenList newTokens;
    int currentTokenPosition = 0;
    for (const TokenPtr& token : allTokens)
    {
        bool tokenHasNewLine = token->type == Token::SPACE && token->value.contains("\n");
        bool commentAfterComment = false;
        bool isFinalToken = allTokens.indexOf(token) == allTokens.size() - 1; // final new-line, any ending comment's position is just before it
        Comment* comment = nullptr;
        while (currentTokenPosition == currentCommentPosition)
        {
            comment = comments.takeFirst();

            if (commentAfterComment || isFinalToken)
                newTokens << TokenPtr::create(Token::SPACE, "\n");

            if (comment->tokensBefore)
                newTokens << TokenPtr::create(Token::SPACE, " ");

            newTokens << TokenPtr::create(Token::COMMENT, comment->contents);

            if (comments.size() > 0)
                currentCommentPosition = comments.first()->position;
            else
                currentCommentPosition = -1;

            commentAfterComment = true;
        }

        if (comment && !comment->tokensAfter && !tokenHasNewLine)
            newTokens << TokenPtr::create(Token::SPACE, "\n");

        newTokens << token;
        if (!token->isWhitespace())
            currentTokenPosition++;
    }

    // Any remaining comments
    for (Comment* cmt : comments)
    {
        newTokens << TokenPtr::create(Token::COMMENT, cmt->contents);
        newTokens << TokenPtr::create(Token::SPACE, "\n");
    }

    newTokens.normalizeWhitespaceTokens();

    if (cfg.SqlEnterpriseFormatter.MoveAllCommentsToLineEnd.get())
        newTokens = adjustCommentsToEnd(newTokens);
    else
        newTokens = wrapOnlyComments(newTokens);

    // Split by lines and reindex token positions according to line start for easier comment formatting
    QList<TokenList> lines = tokensByLines(newTokens, true);
    for (TokenList& line : lines)
        line.reindexPositions();

    formatEndLineComments(newTokens, lines);
    indentMultiLineComments(newTokens, lines);

    return newTokens.detokenize();
}
