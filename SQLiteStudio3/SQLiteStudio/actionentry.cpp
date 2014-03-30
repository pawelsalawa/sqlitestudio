#include "actionentry.h"
#include "iconmanager.h"

ActionEntry::ActionEntry(const DbTree::Action& action)
{
    this->action = action;
    type = Type::SINGLE;
}

ActionEntry::ActionEntry(const QString &subMenuLabel)
{
    this->subMenuLabel = subMenuLabel;
    type = Type::SUB_MENU;
}

ActionEntry::ActionEntry(const Icon& icon, const QString &subMenuLabel)
{
    this->subMenuLabel = subMenuLabel;
    this->subMenuIcon = icon;
    type = Type::SUB_MENU;
}

ActionEntry::ActionEntry(const QIcon &icon, const QString &subMenuLabel)
{
    this->subMenuLabel = subMenuLabel;
    this->subMenuIcon = icon;
    type = Type::SUB_MENU;
}

ActionEntry &ActionEntry::operator =(const DbTree::Action& action)
{
    this->action = action;
    type = Type::SINGLE;
    return *this;
}

ActionEntry &ActionEntry::operator +=(const DbTree::Action& action)
{
    this->actions += action;
    type = Type::SUB_MENU;
    return *this;
}
