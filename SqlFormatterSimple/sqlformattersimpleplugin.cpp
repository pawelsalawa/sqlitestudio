#include "sqlformattersimpleplugin.h"

CFG_DEFINE(SqlFormatterSimpleConfig)

SqlFormatterSimplePlugin::SqlFormatterSimplePlugin()
{
}

QString SqlFormatterSimplePlugin::format(SqliteQueryPtr query)
{
    TokenList tokens = query->tokens;
    foreach (TokenPtr token, tokens)
    {
        if (token->type == Token::KEYWORD && SIMPLE_FMT_CFG.SqlFormatterSimple.UpperCaseKeywords.get())
            token->value = token->value.toUpper();

        if (token->type == Token::SPACE && SIMPLE_FMT_CFG.SqlFormatterSimple.TrimLongSpaces.get() &&
                token->value.length() > 1)
            token->value = " ";
    }

    return tokens.detokenize();
}

QString SqlFormatterSimplePlugin::getConfigTitle() const
{
    return tr("Simple", "sql formatter plugin");
}
