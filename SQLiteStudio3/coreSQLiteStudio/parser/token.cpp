#include "token.h"
#include "lexer.h"
#include <stdio.h>
#include <QStringList>

Token::Token()
    : lemonType(0), type(INVALID), value(QString()), start(-1), end(-1)
{
}

Token::Token(int lemonType, Type type, QString value, qint64 start, qint64 end)
    : lemonType(lemonType), type(type), value(value), start(start), end(end)
{}

Token::Token(int lemonType, Type type, QChar value, qint64 start, qint64 end)
    : lemonType(lemonType), type(type), value(value), start(start), end(end)
{}

Token::Token(int lemonType, Token::Type type, QString value)
    : lemonType(lemonType), type(type), value(value), start(-1), end(-1)
{
}

Token::Token(QString value)
    : lemonType(0), type(INVALID), value(value), start(0), end(0)
{}

Token::Token(Token::Type type, QString value)
    : lemonType(0), type(type), value(value), start(0), end(0)
{
}

Token::Token(Token::Type type, QString value, qint64 start, qint64 end)
    : lemonType(0), type(type), value(value), start(start), end(end)
{
}

Token::~Token()
{
}

QString Token::toString()
{
    return "{" +
            typeToString(type) +
            " " +
            value +
            " " +
            QString::number(start) +
            " " +
            QString::number(end) +
            "}";
}

const QString Token::typeToString(Token::Type type)
{
    switch (type)
    {
        case Token::CTX_ROWID_KW:
            return "CTX_ROWID_KW";
        case Token::CTX_STRICT_KW:
            return "CTX_STRICT_KW";
        case Token::CTX_NEW_KW:
            return "CTX_NEW_KW";
        case Token::CTX_OLD_KW:
            return "CTX_OLD_KW";
        case Token::CTX_TABLE_NEW:
            return "CTX_TABLE_NEW";
        case Token::CTX_INDEX_NEW:
            return "CTX_INDEX_NEW";
        case Token::CTX_VIEW_NEW:
            return "CTX_VIEW_NEW";
        case Token::CTX_TRIGGER_NEW:
            return "CTX_TRIGGER_NEW";
        case Token::CTX_ALIAS:
            return "CTX_ALIAS";
        case Token::CTX_TRANSACTION:
            return "CTX_transaction";
        case Token::CTX_COLUMN_NEW:
            return "CTX_COLUMN_NEW";
        case Token::CTX_COLUMN_TYPE:
            return "CTX_COLUMN_TYPE";
        case Token::CTX_CONSTRAINT:
            return "CTX_CONSTRAINT";
        case Token::CTX_FK_MATCH:
            return "CTX_FK_MATCH";
        case Token::CTX_PRAGMA:
            return "CTX_PRAGMA";
        case Token::CTX_ERROR_MESSAGE:
            return "CTX_ERROR_MESSAGE";
        case Token::CTX_COLUMN:
            return "CTX_COLUMN";
        case Token::CTX_TABLE:
            return "CTX_TABLE";
        case Token::CTX_DATABASE:
            return "CTX_DATABASE";
        case Token::CTX_FUNCTION:
            return "CTX_FUNCTION";
        case Token::CTX_COLLATION:
            return "CTX_COLLATION";
        case Token::CTX_INDEX:
            return "CTX_INDEX";
        case Token::CTX_TRIGGER:
            return "CTX_TRIGGER";
        case Token::CTX_VIEW:
            return "CTX_VIEW";
        case Token::CTX_JOIN_OPTS:
            return "CTX_JOIN_OPTS";
        case Token::INVALID:
            return "INVALID";
        case Token::OTHER:
            return "OTHER";
        case Token::STRING:
            return "STRING";
        case Token::COMMENT:
            return "COMMENT";
        case Token::FLOAT:
            return "FLOAT";
        case Token::INTEGER:
            return "INTEGER";
        case Token::BIND_PARAM:
            return "BIND_PARAM";
        case Token::OPERATOR:
            return "OPERATOR";
        case Token::PAR_LEFT:
            return "PAR_LEFT";
        case Token::PAR_RIGHT:
            return "PAR_RIGHT";
        case Token::SPACE:
            return "SPACE";
        case Token::BLOB:
            return "BLOB";
        case Token::KEYWORD:
            return "KEYWORD";
    }

    return "";
}

Range Token::getRange()
{
    return Range(start, end);
}

bool Token::isWhitespace(bool includeComments) const
{
    return (type == SPACE || (includeComments && type == COMMENT));
}

bool Token::isSeparating() const
{
    switch (type)
    {
        case Token::SPACE:
        case Token::PAR_LEFT:
        case Token::PAR_RIGHT:
        case Token::OPERATOR:
            return true;
        default:
            break;
    }
    return false;
}

bool Token::isMeaningful() const
{
    switch (type)
    {
        case Token::BIND_PARAM:
        case Token::PAR_LEFT:
        case Token::PAR_RIGHT:
        case Token::BLOB:
        case Token::FLOAT:
        case Token::INTEGER:
        case Token::INVALID:
        case Token::KEYWORD:
        case Token::OTHER:
        case Token::STRING:
        case Token::OPERATOR:
            return true;
        default:
            break;
    }
    return false;
}

bool Token::isDbObjectType() const
{
    return ((type & TOKEN_TYPE_MASK_DB_OBJECT) == TOKEN_TYPE_MASK_DB_OBJECT);
}

QString Token::typeString() const
{
    return typeToString(type);
}

int Token::operator ==(const Token &other)
{
    return type == other.type && value == other.value && start == other.start && end == other.end;
}

bool Token::operator <(const Token &other) const
{
    if (start == other.start)
        return end < other.end;
    else
        return start < other.start;
}

size_t qHash(const TokenPtr& token)
{
    // This doesn't look nice, but it's good enough to satisfy a hash table.
    // It's fast and quite distinguishable.
    // It's rare to have two pointers with the same int type representation,
    // and if that happens, there's always a comparision operator.
    return (size_t)reinterpret_cast<qint64>(token.data());
}

TokenList::TokenList()
    : QList<TokenPtr>()
{
}

TokenList::TokenList(const QList<TokenPtr>& other)
    : QList<TokenPtr>(other)
{
}

QString TokenList::toString() const
{
    return toStringList().join(" ");
}

QStringList TokenList::toStringList() const
{
    QStringList strList;
    for (const TokenPtr& t : *this)
        strList << t->toString();

    return strList;
}

QStringList TokenList::toValueList() const
{
    QStringList strList;
    for (const TokenPtr& t : *this)
        strList << t->value;

    return strList;
}

int TokenList::indexOf(TokenPtr token) const
{
    return QList<TokenPtr>::indexOf(token);
}

int TokenList::indexOf(Token::Type type) const
{
    int i;
    findFirst(type, &i);
    return i;
}

int TokenList::indexOf(Token::Type type, const QString &value, Qt::CaseSensitivity caseSensitivity) const
{
    int i;
    findFirst(type, value, caseSensitivity, &i);
    return i;
}

int TokenList::indexOf(const QString &value, Qt::CaseSensitivity caseSensitivity) const
{
    int i;
    findFirst(value, caseSensitivity, &i);
    return i;
}

int TokenList::lastIndexOf(TokenPtr token) const
{
    return QList<TokenPtr>::lastIndexOf(token);
}

int TokenList::lastIndexOf(Token::Type type) const
{
    int i;
    findLast(type, &i);
    return i;
}

int TokenList::lastIndexOf(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity) const
{
    int i;
    findLast(type, value, caseSensitivity, &i);
    return i;
}

int TokenList::lastIndexOf(const QString& value, Qt::CaseSensitivity caseSensitivity) const
{
    int i;
    findLast(value, caseSensitivity, &i);
    return i;
}

TokenPtr TokenList::find(Token::Type type) const
{
    return findFirst(type, nullptr);
}

TokenPtr TokenList::find(Token::Type type, const QString &value, Qt::CaseSensitivity caseSensitivity) const
{
    return findFirst(type, value, caseSensitivity, nullptr);
}

TokenPtr TokenList::find(const QString &value, Qt::CaseSensitivity caseSensitivity) const
{
    return findFirst(value, caseSensitivity, nullptr);
}

TokenPtr TokenList::findLast(Token::Type type) const
{
    return findLast(type, nullptr);
}

TokenPtr TokenList::findLast(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity) const
{
    return findLast(type, value, caseSensitivity, nullptr);
}

TokenPtr TokenList::findLast(const QString& value, Qt::CaseSensitivity caseSensitivity) const
{
    return findLast(value, caseSensitivity, nullptr);
}

TokenPtr TokenList::atCursorPosition(quint64 cursorPosition) const
{
    for (TokenPtr token : *this)
    {
        if (token->getRange().contains(cursorPosition))
            return token;
    }
    return TokenPtr();
}

void TokenList::insert(int i, const TokenList &list)
{
    for (TokenPtr token : list)
        QList<TokenPtr>::insert(i++, token);
}

void TokenList::insert(int i, TokenPtr token)
{
    QList<TokenPtr>::insert(i, token);
}

TokenList &TokenList::operator =(const QList<TokenPtr> &other)
{
    QList<TokenPtr>::operator =(other);
    return *this;
}

QString TokenList::detokenize() const
{
    return Lexer::detokenize(*this);
}

void TokenList::replace(int startIdx, int length, const TokenList& newTokens)
{
    for (int i = 0; i < length; i++)
        removeAt(startIdx);

    insert(startIdx, newTokens);
}

void TokenList::replace(int startIdx, int length, TokenPtr newToken)
{
    for (int i = 0; i < length; i++)
        removeAt(startIdx);

    insert(startIdx, newToken);
}

void TokenList::replace(int startIdx, TokenPtr newToken)
{
    QList<TokenPtr>::replace(startIdx, newToken);
}

void TokenList::replace(int startIdx, const TokenList& newTokens)
{
    replace(startIdx, 1, newTokens);
}

int TokenList::replace(TokenPtr startToken, TokenPtr endToken, const TokenList& newTokens)
{
    int startIdx = indexOf(startToken);
    if (startIdx < 0)
        return 0;

    int endIdx = indexOf(endToken);
    if (endIdx < 0)
        return 0;

    replace(startIdx, endIdx - startIdx, newTokens);
    return endIdx - startIdx;
}

int TokenList::replace(TokenPtr startToken, TokenPtr endToken, TokenPtr newToken)
{
    int startIdx = indexOf(startToken);
    if (startIdx < 0)
        return 0;

    int endIdx = indexOf(endToken);
    if (endIdx < 0)
        return 0;

    replace(startIdx, endIdx - startIdx, newToken);
    return endIdx - startIdx;
}

bool TokenList::replace(TokenPtr oldToken, TokenPtr newToken)
{
    int idx = indexOf(oldToken);
    if (idx < 0)
        return false;

    replace(idx, newToken);
    return true;
}

bool TokenList::replace(TokenPtr oldToken, const TokenList& newTokens)
{
    int idx = indexOf(oldToken);
    if (idx < 0)
        return false;

    replace(idx, newTokens);
    return true;
}

bool TokenList::remove(TokenPtr startToken, TokenPtr endToken)
{
    int startIdx = indexOf(startToken);
    if (startIdx < 0)
        return false;

    int endIdx = indexOf(endToken);
    if (endIdx < 0)
        return false;

    if (endIdx < startIdx)
        return false;

    for (int i = startIdx; i < endIdx; i++)
        removeAt(startIdx);

    return true;
}

bool TokenList::remove(Token::Type type)
{
    int idx = indexOf(type);
    if (idx == -1)
        return false;

    removeAt(idx);
    return true;
}

TokenList& TokenList::trimLeft()
{
    while (size() > 0 && first()->isWhitespace())
        removeFirst();

    return *this;
}

TokenList& TokenList::trimRight()
{
    while (size() > 0 && last()->isWhitespace())
        removeLast();

    return *this;
}

TokenList& TokenList::trim()
{
    trimLeft();
    trimRight();
    return *this;
}

TokenList& TokenList::trimLeft(Token::Type type, const QString& alsoTrim)
{
    while (size() > 0 && (first()->isWhitespace() || (first()->type == type && first()->value == alsoTrim)))
        removeFirst();

    return *this;
}

TokenList& TokenList::trimRight(Token::Type type, const QString& alsoTrim)
{
    while (size() > 0 && (last()->isWhitespace() || (last()->type == type && last()->value == alsoTrim)))
        removeLast();

    return *this;
}

TokenList& TokenList::trim(Token::Type type, const QString& alsoTrim)
{
    trimLeft(type, alsoTrim);
    trimRight(type, alsoTrim);
    return *this;
}

TokenList TokenList::filter(Token::Type type) const
{
    TokenList filtered;
    for (TokenPtr token : *this)
        if (token->type == type)
            filtered << token;

    return filtered;
}

TokenList TokenList::filterOut(Token::Type type) const
{
    TokenList filtered;
    for (TokenPtr token : *this)
        if (token->type != type)
            filtered << token;

    return filtered;
}

TokenList TokenList::filterWhiteSpaces(bool includeComments) const
{
    TokenList filtered;
    for (TokenPtr token : *this)
        if (!token->isWhitespace(includeComments))
            filtered << token;

    return filtered;
}

TokenList TokenList::mid(int pos, int length) const
{
    TokenList newList = QList<TokenPtr>::mid(pos, length);
    return newList;
}

TokenPtr TokenList::findFirst(Token::Type type, int *idx) const
{
    int i = -1;
    TokenPtr token;
    QListIterator<TokenPtr> it(*this);
    while (it.hasNext())
    {
        token = it.next();
        i++;
        if (token->type == type)
        {
            if (idx) (*idx) = i;
            return token;
        }
    }
    if (idx) (*idx) = -1;
    return TokenPtr();
}

TokenPtr TokenList::findFirst(Token::Type type, const QString &value, Qt::CaseSensitivity caseSensitivity, int *idx) const
{
    int i = -1;
    TokenPtr token;
    QListIterator<TokenPtr> it(*this);
    while (it.hasNext())
    {
        token = it.next();
        i++;
        if (token->type != type)
            continue;

        if (token->value.compare(value, caseSensitivity) == 0)
        {
            if (idx) (*idx) = i;
            return token;
        }
    }
    if (idx) (*idx) = -1;
    return TokenPtr();
}

TokenPtr TokenList::findFirst(const QString &value, Qt::CaseSensitivity caseSensitivity, int *idx) const
{
    int i = -1;
    TokenPtr token;
    QListIterator<TokenPtr> it(*this);
    while (it.hasNext())
    {
        token = it.next();
        i++;
        if (token->value.compare(value, caseSensitivity) == 0)
        {
            if (idx) (*idx) = i;
            return token;
        }
    }
    if (idx) (*idx) = -1;
    return TokenPtr();
}


TokenPtr TokenList::findLast(Token::Type type, int* idx) const
{
    int i = size();
    TokenPtr token;
    QListIterator<TokenPtr> it(*this);
    it.toBack();
    while (it.hasPrevious())
    {
        token = it.previous();
        i--;
        if (token->type == type)
        {
            if (idx) (*idx) = i;
            return token;
        }
    }
    if (idx) (*idx) = -1;
    return TokenPtr();
}

TokenPtr TokenList::findLast(Token::Type type, const QString& value, Qt::CaseSensitivity caseSensitivity, int* idx) const
{
    int i = size();
    TokenPtr token;
    QListIterator<TokenPtr> it(*this);
    it.toBack();
    while (it.hasPrevious())
    {
        token = it.previous();
        i--;
        if (token->type != type)
            continue;

        if (token->value.compare(value, caseSensitivity) == 0)
        {
            if (idx) (*idx) = i;
            return token;
        }
    }
    if (idx) (*idx) = -1;
    return TokenPtr();
}

TokenPtr TokenList::findLast(const QString& value, Qt::CaseSensitivity caseSensitivity, int* idx) const
{
    int i = size();
    TokenPtr token;
    QListIterator<TokenPtr> it(*this);
    it.toBack();
    while (it.hasPrevious())
    {
        token = it.previous();
        i--;
        if (token->value.compare(value, caseSensitivity) == 0)
        {
            if (idx) (*idx) = i;
            return token;
        }
    }
    if (idx) (*idx) = -1;
    return TokenPtr();
}
