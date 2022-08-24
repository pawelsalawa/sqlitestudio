#include "bistrhash.h"
#include <QStringList>

BiStrHash::BiStrHash(std::initializer_list<std::pair<QString, QString> > list)
{
    hash = QHash<QString,QString>(list);
    initInvertedAndLower();
}

BiStrHash::BiStrHash(const QHash<QString, QString>& other)
{
    unite(other);
}

BiStrHash::BiStrHash(const BiStrHash& other) : hash(other.hash), inverted(other.inverted),
    lowerHash(other.lowerHash), lowerInverted(other.lowerInverted)
{
}

void BiStrHash::insert(const QString& left, const QString& right)
{
    if (lowerHash.contains(left.toLower()))
        removeLeft(left, Qt::CaseInsensitive);

    if (lowerInverted.contains(right.toLower()))
        removeRight(right, Qt::CaseInsensitive);

    inverted.insert(right, left);
    hash.insert(left, right);
    lowerHash.insert(left.toLower(), left);
    lowerInverted.insert(right.toLower(), right);
}

bool BiStrHash::containsLeft(const QString& left, Qt::CaseSensitivity cs) const
{
    if (cs == Qt::CaseSensitive)
        return hash.contains(left);
    else
        return lowerHash.contains(left.toLower());
}

bool BiStrHash::containsRight(const QString& right, Qt::CaseSensitivity cs) const
{
    if (cs == Qt::CaseSensitive)
        return inverted.contains(right);
    else
        return lowerInverted.contains(right.toLower());
}

int BiStrHash::removeLeft(const QString& left, Qt::CaseSensitivity cs)
{
    if (cs == Qt::CaseSensitive)
    {
        if (!hash.contains(left))
            return 0;

        inverted.remove(hash.value(left));
        hash.remove(left);

        return 1;
    }
    else
    {
        QString lowerLeft = left.toLower();
        if (!lowerHash.contains(lowerLeft))
            return 0;

        QString right = hash.value(lowerHash.value(lowerLeft));

        hash.remove(inverted.value(right));
        inverted.remove(right);
        lowerHash.remove(lowerLeft);
        lowerInverted.remove(right.toLower());

        return 1;
    }
}

int BiStrHash::removeRight(const QString& right, Qt::CaseSensitivity cs)
{
    if (cs == Qt::CaseSensitive)
    {
        if (!inverted.contains(right))
            return 0;

        hash.remove(inverted.value(right));
        inverted.remove(right);

        return 1;
    }
    else
    {
        QString lowerRight = right.toLower();
        if (!lowerInverted.contains(lowerRight))
            return 0;

        QString left = inverted.value(lowerInverted.value(lowerRight));

        inverted.remove(hash.value(left));
        hash.remove(left);
        lowerHash.remove(left.toLower());
        lowerInverted.remove(lowerRight);

        return 1;
    }
}

QString BiStrHash::takeLeft(const QString& left, Qt::CaseSensitivity cs)
{
    if (cs == Qt::CaseSensitive)
    {
        QString right = hash.take(left);
        inverted.remove(right);
        return right;
    }
    else
    {
        QString right = hash.take(lowerHash.take(left.toLower()));
        inverted.remove(lowerInverted.take(right.toLower()));
        return right;
    }
}

QString BiStrHash::takeRight(const QString& right, Qt::CaseSensitivity cs)
{
    if (cs == Qt::CaseSensitive)
    {
        QString left = inverted.take(right);
        hash.remove(left);
        return left;
    }
    else
    {
        QString left = inverted.take(lowerInverted.take(right.toLower()));
        hash.remove(lowerHash.take(left.toLower()));
        return left;
    }
}

BiStrHash& BiStrHash::unite(const BiStrHash& other)
{
    unite(other.hash);
    return *this;
}

BiStrHash& BiStrHash::unite(const QHash<QString, QString>& other)
{
    QHashIterator<QString, QString> it(other);
    while (it.hasNext())
    {
        it.next();
        insert(it.key(), it.value());
    }

    return *this;
}

QString BiStrHash::valueByLeft(const QString& left, Qt::CaseSensitivity cs) const
{
    if (cs == Qt::CaseSensitive)
        return hash.value(left);
    else
        return hash.value(lowerHash.value(left.toLower()));
}

QString BiStrHash::valueByLeft(const QString& left, const QString& defaultValue, Qt::CaseSensitivity cs) const
{
    if (containsLeft(left, cs))
        return valueByLeft(left, cs);

    return defaultValue;
}

QString BiStrHash::valueByRight(const QString& right, Qt::CaseSensitivity cs) const
{
    if (cs == Qt::CaseSensitive)
        return inverted.value(right);
    else
        return inverted.value(lowerInverted.value(right.toLower()));
}

QString BiStrHash::valueByRight(const QString& right, const QString& defaultValue, Qt::CaseSensitivity cs) const
{
    if (containsRight(right, cs))
        return valueByRight(right, cs);

    return defaultValue;
}

QStringList BiStrHash::leftValues() const
{
    return hash.keys();
}

QStringList BiStrHash::rightValues() const
{
    return inverted.keys();
}

QHashIterator<QString, QString> BiStrHash::iterator() const
{
    return QHashIterator<QString,QString>(hash);
}

void BiStrHash::clear()
{
    hash.clear();
    inverted.clear();
    lowerHash.clear();
    lowerInverted.clear();
}

int BiStrHash::count() const
{
    return hash.count();
}

bool BiStrHash::isEmpty() const
{
    return hash.isEmpty();
}

void BiStrHash::initInvertedAndLower()
{
    QHashIterator<QString,QString> it(hash);
    while (it.hasNext())
    {
        it.next();
        inverted[it.value()] = it.key();
        lowerHash[it.key().toLower()] = it.key();
        lowerInverted[it.value().toLower()] = it.value();
    }
}
