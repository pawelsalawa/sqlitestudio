#ifndef ACTIONENTRY_H
#define ACTIONENTRY_H

#include "dbtree/dbtree.h"
#include <QString>
#include <QIcon>

struct GUI_API_EXPORT ActionEntry
{
    enum class Type
    {
        SINGLE,
        SUB_MENU
    };

    QString subMenuLabel;
    QIcon subMenuIcon;
    QList<DbTree::Action> actions;
    DbTree::Action action;
    Type type;

    ActionEntry(const DbTree::Action &action);
    ActionEntry(const QString& subMenuLabel);
    ActionEntry(const Icon& icon, const QString& subMenuLabel);
    ActionEntry(const QIcon& icon, const QString& subMenuLabel);
    ActionEntry& operator=(const DbTree::Action& action);
    ActionEntry& operator+=(const DbTree::Action& action);
};

#endif // ACTIONENTRY_H
