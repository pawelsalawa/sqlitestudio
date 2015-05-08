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

    QString formattedWithComments = applyComments(formatted, comments, query->dialect);
    for (Comment* c : comments)
        delete c;

    return formattedWithComments;
}

bool SqlEnterpriseFormatter::init()
{
    Q_INIT_RESOURCE(sqlenterpriseformatter);

    static_qstring(query1, "SELECT (2 + 4) AND (3 + 5), 4 NOT IN (SELECT t1.'some[_]name' + t2.[some'name2] FROM xyz t1 JOIN zxc t2 ON (t1.aaa = t2.aaa)) "
                           "FROM a, (SELECT id FROM table2);");
    static_qstring(query2, "INSERT INTO table1 (id, value1, value2) VALUES (1, (2 + 5), (SELECT id FROM table2));");
    static_qstring(query3, "CREATE TABLE tab (id INTEGER PRIMARY KEY, value1 VARCHAR(6), value2 NUMBER(8,2) NOT NULL DEFAULT 1.0);");
    static_qstring(query4, "CREATE UNIQUE INDEX IF NOT EXISTS dbName.idx1 ON [messages column] (id COLLATE x ASC, lang DESC, description);");

    Parser parser(Dialect::Sqlite3);

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
    Q_CLEANUP_RESOURCE(sqlenterpriseformatter);
}


void SqlEnterpriseFormatter::updatePreview()
{
    QStringList output;
    for (const SqliteQueryPtr& q : previewQueries)
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
                cmt->contents = token->value;
                cmt->position = pos;
                cmt->multiline = token->value.startsWith("/*");
                results << cmt;
                prevCommentInThisLine = cmt;
            }
            tokensBefore = true;
            pos++;
        }
        line++;
    }
}

QList<TokenList> SqlEnterpriseFormatter::tokensByLines(const TokenList &tokens)
{
    QList<TokenList> tokensInLines;
    TokenList tokensInLine;
    for (const TokenPtr& token : tokens)
    {
        if (token->type != Token::Type::SPACE)
        {
            tokensInLine << token;
            continue;
        }

        if (token->value.contains('\n'))
        {
            tokensInLines << tokensInLine;
            tokensInLine.clear();
        }
    }
    return tokensInLines;
}

QString SqlEnterpriseFormatter::applyComments(const QString& formatted, const QList<SqlEnterpriseFormatter::Comment*>& comments, Dialect dialect)
{
    TokenList allTokens = Lexer::tokenize(formatted, dialect);
    TokenList tokensWithoutSpaces = allTokens.filterWhiteSpaces(false);

    // Insert comments initially just as their index in comments collection
    int pos = 0;
    for (Comment* cmt : comments)
        tokensWithoutSpaces.insert(cmt->position, TokenPtr::create(Token::Type::COMMENT, QString::number(pos++)));

//    QList<TokenList> tokensInLines = tokensByLines(tokensWithoutSpaces);
    return tokensWithoutSpaces.detokenize();
}
