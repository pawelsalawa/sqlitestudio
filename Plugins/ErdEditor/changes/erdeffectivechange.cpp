#include "erdeffectivechange.h"
#include <QUuid>

ErdEffectiveChange::ErdEffectiveChange(Type type, const QString& description) :
    type(type), description(description)
{
    id = QUuid::createUuid().toString();
}

QString ErdEffectiveChange::getId() const
{
    return id;
}

QString ErdEffectiveChange::getDescription() const
{
    return description;
}

bool ErdEffectiveChange::isValid() const
{
    return type != Type::INVALID && type != Type::NOOP;
}

SqliteCreateTablePtr ErdEffectiveChange::getAfter() const
{
    return after;
}

SqliteCreateTablePtr ErdEffectiveChange::getBefore() const
{
    return before;
}

QString ErdEffectiveChange::getTableName() const
{
    switch (type)
    {
        case Type::CREATE:
            return after->table;
        case Type::DROP:
            return tableName;
        case Type::MODIFY:
            return before->table;
        case Type::INVALID:
            break;
    }
    qCritical() << "ErdEffectiveChange::getTableName: Unknown type" << static_cast<int>(type)
                << ", desc:" << description;
    return QString();
}

ErdEffectiveChange::Type ErdEffectiveChange::getType() const
{
    return type;
}

ErdEffectiveChange ErdEffectiveChange::noop()
{
    return ErdEffectiveChange(Type::NOOP, QString());
}

ErdEffectiveChange ErdEffectiveChange::drop(const QString& tableName, const QString& description)
{
    ErdEffectiveChange change(Type::DROP, description);
    change.tableName = tableName;
    return change;
}

ErdEffectiveChange ErdEffectiveChange::create(const SqliteCreateTablePtr& after, const QString& description)
{
    ErdEffectiveChange change(Type::CREATE, description);
    change.after = after;
    return change;
}

ErdEffectiveChange ErdEffectiveChange::modify(const SqliteCreateTablePtr& before, const SqliteCreateTablePtr& after, const QString& description)
{
    ErdEffectiveChange change(Type::MODIFY, description);
    change.before = before;
    change.after = after;
    return change;
}
