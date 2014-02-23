#ifndef ACTIONENTRY_H
#define ACTIONENTRY_H

#include "dbtree/dbtree.h"
#include <QString>
#include <QIcon>

struct ActionEntry
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
    ActionEntry(const QString& icon, const QString& subMenuLabel);
    ActionEntry(const QIcon& icon, const QString& subMenuLabel);
    ActionEntry& operator=(const DbTree::Action& action);
    ActionEntry& operator+=(const DbTree::Action& action);
};

#endif // ACTIONENTRY_H
