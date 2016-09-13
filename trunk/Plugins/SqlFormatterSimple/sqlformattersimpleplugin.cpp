#include "sqlformattersimpleplugin.h"

SqlFormatterSimplePlugin::SqlFormatterSimplePlugin()
{
}

QString SqlFormatterSimplePlugin::format(SqliteQueryPtr query)
{
    TokenList tokens = query->tokens;
    tokens.trimRight();
    foreach (TokenPtr token, tokens)
    {
        if (token->type == Token::KEYWORD && cfg.SqlFormatterSimple.UpperCaseKeywords.get())
            token->value = token->value.toUpper();

        if (token->type == Token::SPACE && cfg.SqlFormatterSimple.TrimLongSpaces.get() &&
                token->value.length() > 1)
            token->value = " ";
    }

    if (!tokens.isEmpty())
    {
        TokenPtr last = tokens.last();
        if (last->type != Token::Type::OPERATOR || last->value != ";")
            tokens << TokenPtr::create(Token::Type::OPERATOR, ";");
    }

    return tokens.detokenize();
}

bool SqlFormatterSimplePlugin::init()
{
    Q_INIT_RESOURCE(sqlformattersimple);
    return GenericPlugin::init();
}

void SqlFormatterSimplePlugin::deinit()
{
    Q_CLEANUP_RESOURCE(sqlformattersimple);
}

QString SqlFormatterSimplePlugin::getConfigUiForm() const
{
    return "SqlFormatterSimplePlugin";
}

CfgMain* SqlFormatterSimplePlugin::getMainUiConfig()
{
    return &cfg;
}

void SqlFormatterSimplePlugin::configDialogOpen()
{
}

void SqlFormatterSimplePlugin::configDialogClosed()
{
}
