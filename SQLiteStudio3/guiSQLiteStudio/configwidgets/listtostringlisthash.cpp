#include "listtostringlisthash.h"
#include "config_builder.h"
#include "common/unused.h"
#include <QListWidget>

ListToStringListHash::ListToStringListHash(CfgEntry* key) :
    assignedKey(key)
{
}

bool ListToStringListHash::isConfigForWidget(CfgEntry* key, QWidget* widget)
{
    UNUSED(widget);
    return (assignedKey == key);
}

void ListToStringListHash::applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value)
{
    UNUSED(key);

    QListWidget* list = dynamic_cast<QListWidget*>(widget);
    QHash<QString,QListWidgetItem*> itemsByName;
    for (int i = 0; i < list->count(); i++)
        itemsByName[list->item(i)->text()] = list->item(i);

    QHash<QString,QVariant> orderHash = value.toHash();

    for (const QString& typeName : itemsByName.keys())
    {
        if (!orderHash.contains(typeName))
            continue;

        itemsByName[typeName]->setData(QListWidgetItem::UserType, orderHash[typeName]);
    }
}

void ListToStringListHash::saveWidgetToConfig(QWidget* widget, CfgEntry* key)
{
    QListWidget* list = dynamic_cast<QListWidget*>(widget);
    QHash<QString,QVariant> orderHash;
    for (int i = 0; i < list->count(); i++)
        orderHash[list->item(i)->text()] = list->item(i)->data(QListWidgetItem::UserType);

    key->set(orderHash);
}

const char*ListToStringListHash::getModifiedNotifier() const
{
    return SIGNAL(itemChanged(QListWidgetItem*));
}

QString ListToStringListHash::getFilterString(QWidget* widget) const
{
    QListWidget* list = dynamic_cast<QListWidget*>(widget);
    QStringList strList;
    for (int i = 0; i < list->count(); i++)
        strList += list->item(i)->text() + " " + list->item(i)->data(QListWidgetItem::UserType).toStringList().join(" ");

    return strList.join(" ");
}
