#include "sqlitesyntaxhighlighter.h"
#include "parser/lexer.h"
#include "uiconfig.h"
#include "services/config.h"
#include <QTextDocument>
#include <QDebug>
#include <QPlainTextEdit>

SqliteSyntaxHighlighter::SqliteSyntaxHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    setupFormats();
    setupMapping();
    setCurrentBlockState(regulartTextBlockState);
    connect(CFG, SIGNAL(massSaveCommited()), this, SLOT(setupFormats()));
}

void SqliteSyntaxHighlighter::setSqliteVersion(int version)
{
    this->sqliteVersion = version;
    rehighlight();
}

void SqliteSyntaxHighlighter::setFormat(SqliteSyntaxHighlighter::State state, QTextCharFormat format)
{
    formats[state] = format;
}

QTextCharFormat SqliteSyntaxHighlighter::getFormat(SqliteSyntaxHighlighter::State state) const
{
    return formats[state];
}

void SqliteSyntaxHighlighter::setupFormats()
{
    QTextCharFormat format;

    // Standard
    format.setForeground(CFG_UI.Colors.SqlEditorForeground.get());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(false);
    formats[State::STANDARD] = format;

    // Parenthesis
    formats[State::PARENTHESIS] = format;

    // String
    format.setForeground(CFG_UI.Colors.SqlEditorStringFg.get());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(false);
    formats[State::STRING] = format;

    // Keyword
    format.setForeground(CFG_UI.Colors.SqlEditorKeywordFg.get());
    format.setFontWeight(QFont::Bold);
    format.setFontItalic(false);
    formats[State::KEYWORD] = format;

    // BindParam
    format.setForeground(CFG_UI.Colors.SqlEditorBindParamFg.get());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(false);
    formats[State::BIND_PARAM] = format;

    // Blob
    format.setForeground(CFG_UI.Colors.SqlEditorBlobFg.get());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(false);
    formats[State::BLOB] = format;

    // Comment
    format.setForeground(CFG_UI.Colors.SqlEditorCommentFg.get());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(true);
    formats[State::COMMENT] = format;

    // Number
    format.setForeground(CFG_UI.Colors.SqlEditorNumberFg.get());
    format.setFontWeight(QFont::Normal);
    format.setFontItalic(false);
    formats[State::NUMBER] = format;
}

void SqliteSyntaxHighlighter::setupMapping()
{
    tokenTypeMapping[Token::STRING] = State::STRING;
    tokenTypeMapping[Token::COMMENT] = State::COMMENT;
    tokenTypeMapping[Token::FLOAT] = State::NUMBER;
    tokenTypeMapping[Token::INTEGER] = State::NUMBER;
    tokenTypeMapping[Token::BIND_PARAM] = State::BIND_PARAM;
    tokenTypeMapping[Token::PAR_LEFT] = State::PARENTHESIS;
    tokenTypeMapping[Token::PAR_RIGHT] = State::PARENTHESIS;
    tokenTypeMapping[Token::BLOB] = State::BLOB;
    tokenTypeMapping[Token::KEYWORD] = State::KEYWORD;
}

QString SqliteSyntaxHighlighter::getPreviousStatePrefix(TextBlockState textBlockState)
{
    QString prefix = "";
    switch (textBlockState)
    {
        case SqliteSyntaxHighlighter::TextBlockState::REGULAR:
            break;
        case SqliteSyntaxHighlighter::TextBlockState::BLOB:
            prefix = "x'";
            break;
        case SqliteSyntaxHighlighter::TextBlockState::STRING:
            prefix = "'";
            break;
        case SqliteSyntaxHighlighter::TextBlockState::COMMENT:
            prefix = "/*";
            break;
        case SqliteSyntaxHighlighter::TextBlockState::ID_1:
            prefix = "[";
            break;
        case SqliteSyntaxHighlighter::TextBlockState::ID_2:
            prefix = "\"";
            break;
        case SqliteSyntaxHighlighter::TextBlockState::ID_3:
            prefix = "`";
            break;
    }
    return prefix;
}

void SqliteSyntaxHighlighter::highlightBlock(const QString &text)
{
    if (text.length() <= 0 || document()->characterCount() > MAX_QUERY_LENGTH)
        return;

    // Reset to default
    QSyntaxHighlighter::setFormat(0, text.length(), formats[State::STANDARD]);

    qint32 idxModifier = 0;
    QString statePrefix = "";
    if (previousBlockState() != regulartTextBlockState)
    {
        statePrefix = getPreviousStatePrefix(static_cast<TextBlockState>(previousBlockState()));
        idxModifier += statePrefix.size();
    }

    Lexer lexer(sqliteVersion == 2 ? Dialect::Sqlite2 : Dialect::Sqlite3);
    lexer.setTolerantMode(true);
    lexer.prepare(statePrefix+text);

    // Previous error state.
    // Empty lines have no userData, so we will look for any previous paragraph that is
    // valid and has a data, so it has any logical meaning to highlighter.
    QTextBlock prevBlock = currentBlock().previous();
    while ((!prevBlock.isValid() || !prevBlock.userData() || prevBlock.text().isEmpty()) && prevBlock.position() > 0)
        prevBlock = prevBlock.previous();

    TextBlockData* prevData = nullptr;
    if (prevBlock.isValid())
        prevData = dynamic_cast<TextBlockData*>(prevBlock.userData());

    TextBlockData* data = new TextBlockData();
    int errorStart = -1;
    TokenPtr token = lexer.getToken();
    while (token)
    {
        if (handleToken(token, idxModifier, errorStart, data, prevData))
            errorStart = token->start + currentBlock().position();

        if (data->getEndsWithQuerySeparator())
            errorStart = -1;

        handleParenthesis(token, data);
        token = lexer.getToken();
    }

    setCurrentBlockUserData(data);
}

bool SqliteSyntaxHighlighter::handleToken(TokenPtr token, qint32 idxModifier, int errorStart, TextBlockData* currBlockData,
                                          TextBlockData* previousBlockData)
{
    qint64 start = token->start - idxModifier;
    qint64 lgt = token->end - token->start + 1;
    if (start < 0)
    {
        lgt += start; // cut length by num of chars before 0 (after idxModifier applied)
        start = 0;
    }

    if (createTriggerContext && token->type == Token::OTHER && (token->value.toLower() == "old" || token->value.toLower() == "new"))
        token->type = Token::KEYWORD;

    bool limitedDamage = false;
    bool querySeparator = (token->type == Token::Type::OPERATOR && token->value == ";");
    bool error = isError(start, lgt, &limitedDamage);
    bool valid = isValid(start, lgt);
    bool wasError = (
                        (errorStart > -1) &&
                        (start + currentBlock().position() + lgt >= errorStart) &&
                        !currBlockData->getEndsWithQuerySeparator() // if it was set for previous token in the same block
                    ) ||
                    (
                        token->start == 0 &&
                        previousBlockData &&
                        previousBlockData->getEndsWithError() &&
                        !previousBlockData->getEndsWithQuerySeparator()
                    );
    bool fatalError = (error && !limitedDamage) || wasError;

    QTextCharFormat format;

    // Applying valid object format.
    applyValidObjectFormat(format, valid, error, wasError);

    // Get format for token type (if any)
    if (tokenTypeMapping.contains(token->type))
        format = formats[tokenTypeMapping[token->type]];

    // Merge with error format (if this is an error).
    applyErrorFormat(format, error, wasError, token->type);

    // Apply format
    QSyntaxHighlighter::setFormat(start, lgt, format);

    // Save block state
    TolerantTokenPtr tolerantToken = token.dynamicCast<TolerantToken>();
    if (tolerantToken->invalid)
        setStateForUnfinishedToken(tolerantToken);
    else
        setCurrentBlockState(regulartTextBlockState);

    currBlockData->setEndsWithError(fatalError);
    currBlockData->setEndsWithQuerySeparator(querySeparator);

    return fatalError;
}

void SqliteSyntaxHighlighter::applyErrorFormat(QTextCharFormat& format, bool isError, bool wasError, Token::Type tokenType)
{
    if ((!isError && !wasError) || tokenType == Token::Type::COMMENT)
        return;

    format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    format.setUnderlineColor(QColor(Qt::red));
}

void SqliteSyntaxHighlighter::applyValidObjectFormat(QTextCharFormat& format, bool isValid, bool isError, bool wasError)
{
    if (isError || wasError || !isValid)
        return;

    format.setForeground(CFG_UI.Colors.SqlEditorValidObject.get());
    if (objectLinksEnabled)
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
}

void SqliteSyntaxHighlighter::handleParenthesis(TokenPtr token, TextBlockData* data)
{
    if (token->type == Token::PAR_LEFT || token->type == Token::PAR_RIGHT)
        data->insertParenthesis(currentBlock().position() + token->start, token->value[0].toLatin1());
}
bool SqliteSyntaxHighlighter::getCreateTriggerContext() const
{
    return createTriggerContext;
}

void SqliteSyntaxHighlighter::setCreateTriggerContext(bool value)
{
    createTriggerContext = value;
}

bool SqliteSyntaxHighlighter::getObjectLinksEnabled() const
{
    return objectLinksEnabled;
}

void SqliteSyntaxHighlighter::setObjectLinksEnabled(bool value)
{
    objectLinksEnabled = value;
}

bool SqliteSyntaxHighlighter::isError(int start, int lgt, bool* limitedDamage)
{
    start += currentBlock().position();
    int end = start + lgt - 1;
    foreach (const Error& error, errors)
    {
        if (error.from <= start && error.to >= end)
        {
            *limitedDamage = error.limitedDamage;
            return true;
        }
    }
    return false;
}

bool SqliteSyntaxHighlighter::isValid(int start, int lgt)
{
    start += currentBlock().position();
    int end = start + lgt - 1;
    foreach (const DbObject& obj, dbObjects)
    {
        if (obj.from <= start && obj.to >= end)
            return true;
    }
    return false;
}

void SqliteSyntaxHighlighter::setStateForUnfinishedToken(TolerantTokenPtr tolerantToken)
{
    switch (tolerantToken->type)
    {
        case Token::OTHER:
        {
            switch (tolerantToken->value.at(0).toLatin1())
            {
                case '[':
                    setCurrentBlockState(static_cast<int>(TextBlockState::ID_1));
                    break;
                case '"':
                    setCurrentBlockState(static_cast<int>(TextBlockState::ID_2));
                    break;
                case '`':
                    setCurrentBlockState(static_cast<int>(TextBlockState::ID_3));
                    break;
            }
            break;
        }
        case Token::STRING:
            setCurrentBlockState(static_cast<int>(TextBlockState::STRING));
            break;
        case Token::COMMENT:
            setCurrentBlockState(static_cast<int>(TextBlockState::COMMENT));
            break;
        case Token::BLOB:
            setCurrentBlockState(static_cast<int>(TextBlockState::BLOB));
            break;
        default:
            break;
    }
}
void SqliteSyntaxHighlighter::clearErrors()
{
    errors.clear();
}

bool SqliteSyntaxHighlighter::haveErrors()
{
    return errors.count() > 0;
}

void SqliteSyntaxHighlighter::addDbObject(int from, int to)
{
    dbObjects << DbObject(from, to);
}

void SqliteSyntaxHighlighter::clearDbObjects()
{
    dbObjects.clear();
}

void SqliteSyntaxHighlighter::addError(int from, int to, bool limitedDamage)
{
    errors << Error(from, to, limitedDamage);
}

SqliteSyntaxHighlighter::Error::Error(int from, int to, bool limitedDamage) :
    from(from), to(to), limitedDamage(limitedDamage)
{
}

int qHash(SqliteSyntaxHighlighter::State state)
{
    return static_cast<int>(state);
}


SqliteSyntaxHighlighter::DbObject::DbObject(int from, int to) :
    from(from), to(to)
{
}

QList<const TextBlockData::Parenthesis*> TextBlockData::parentheses()
{
    QList<const TextBlockData::Parenthesis*> list;
    foreach (const TextBlockData::Parenthesis& par, parData)
        list << &par;

    return list;
}

void TextBlockData::insertParenthesis(int pos, char c)
{
    Parenthesis par;
    par.character = c;
    par.position = pos;
    parData << par;
}

const TextBlockData::Parenthesis* TextBlockData::parenthesisForPosision(int pos)
{
    foreach (const Parenthesis& par, parData)
    {
        if (par.position == pos)
            return &par;
    }
    return nullptr;
}
bool TextBlockData::getEndsWithError() const
{
    return endsWithError;
}

void TextBlockData::setEndsWithError(bool value)
{
    endsWithError = value;
}
bool TextBlockData::getEndsWithQuerySeparator() const
{
    return endsWithQuerySeparator;
}

void TextBlockData::setEndsWithQuerySeparator(bool value)
{
    endsWithQuerySeparator = value;
}


int TextBlockData::Parenthesis::operator==(const TextBlockData::Parenthesis& other)
{
    return other.position == position && other.character == character;
}

QString SqliteHighlighterPlugin::getLanguageName() const
{
    return "SQL";
}

QSyntaxHighlighter* SqliteHighlighterPlugin::createSyntaxHighlighter(QWidget* textEdit) const
{
    QPlainTextEdit* plainEdit = dynamic_cast<QPlainTextEdit*>(textEdit);
    if (plainEdit)
        return new SqliteSyntaxHighlighter(plainEdit->document());

    QTextEdit* edit = dynamic_cast<QTextEdit*>(textEdit);
    if (edit)
        return new SqliteSyntaxHighlighter(edit->document());

    return nullptr;
}
