#include "expectedtoken.h"

bool ExpectedToken::needsWrapping() const
{
    switch (type)
    {
        case ExpectedToken::COLUMN:
        case ExpectedToken::TABLE:
        case ExpectedToken::INDEX:
        case ExpectedToken::TRIGGER:
        case ExpectedToken::VIEW:
        case ExpectedToken::DATABASE:
        case ExpectedToken::OTHER:
        case ExpectedToken::COLLATION:
            return true;
        case ExpectedToken::KEYWORD:
        case ExpectedToken::FUNCTION:
        case ExpectedToken::OPERATOR:
        case ExpectedToken::STRING:
        case ExpectedToken::NUMBER:
        case ExpectedToken::BLOB:
        case ExpectedToken::PRAGMA:
        case ExpectedToken::NO_VALUE:
            return false;
    }
    return false;
}

int ExpectedToken::operator==(const ExpectedToken& other) const
{
    return type == other.type && value == other.value && contextInfo == other.contextInfo &&
            label == other.label && prefix == other.prefix;
}

QString ExpectedToken::toString() const
{
    return QString("%4. %1 : %2 (ctx: %3) [label: %5]").arg(value).arg(type).arg(contextInfo).arg(prefix).arg(label);
}

ExpectedTokenPtr::ExpectedTokenPtr() :
    QSharedPointer<ExpectedToken>()
{
}

ExpectedTokenPtr::ExpectedTokenPtr(ExpectedToken* ptr) :
    QSharedPointer<ExpectedToken>(ptr)
{
}

ExpectedTokenPtr::ExpectedTokenPtr(const QSharedPointer<ExpectedToken>& other) :
    QSharedPointer<ExpectedToken>(other)
{
}

ExpectedTokenPtr::ExpectedTokenPtr(const QWeakPointer<ExpectedToken>& other) :
    QSharedPointer<ExpectedToken>(other)
{
}

int operator==(const ExpectedTokenPtr& ptr1, const ExpectedTokenPtr& ptr2)
{
    return *ptr1.data() == *ptr2.data();
}

size_t qHash(const ExpectedToken& token)
{
    return token.type ^ qHash(token.value + "/" + token.contextInfo + "/" + token.label + "/" + token.prefix);
}

size_t qHash(const ExpectedTokenPtr& ptr)
{
    return qHash(*ptr.data());
}
