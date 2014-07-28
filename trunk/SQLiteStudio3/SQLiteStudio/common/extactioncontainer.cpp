#include "extactioncontainer.h"
#include "iconmanager.h"
#include "common/extaction.h"
#include "common/global.h"
#include "extactioncontainersignalhandler.h"
#include <QSignalMapper>
#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QDebug>

ExtActionContainer::ExtActionContainer()
{
    actionIdMapper = new QSignalMapper();
    signalHandler = new ExtActionContainerSignalHandler(this);
    QObject::connect(actionIdMapper, SIGNAL(mapped(int)), signalHandler, SLOT(handleShortcutChange(int)));
}

ExtActionContainer::~ExtActionContainer()
{
    deleteActions();
    safe_delete(signalHandler);
    safe_delete(actionIdMapper);
}

void ExtActionContainer::initActions()
{
    createActions();
    setupDefShortcuts();
    refreshShortcuts();
}

void ExtActionContainer::createAction(int action, const Icon& icon, const QString& text, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    QAction* qAction = new ExtAction(icon, text);
    createAction(action, qAction, receiver, slot, container, owner);
}

void ExtActionContainer::createAction(int action, const QString& text, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    QAction* qAction = new ExtAction(text);
    createAction(action, qAction, receiver, slot, container, owner);
}

void ExtActionContainer::bindShortcutsToEnum(CfgCategory &cfgCategory, const QMetaEnum &actionsEnum)
{
    QHash<QString, CfgEntry *>& cfgEntries = cfgCategory.getEntries();
    QString enumName;
    CfgStringEntry* stringEntry = nullptr;
    for (int i = 0, total = actionsEnum.keyCount(); i < total; ++i)
    {
        enumName = QString::fromLatin1(actionsEnum.key(i));
        if (!cfgEntries.contains(enumName))
            continue;

        stringEntry = dynamic_cast<CfgStringEntry*>(cfgEntries[enumName]);
        if (!stringEntry)
        {
            qDebug() << "Tried to bind key sequence config entry, but its type was not QString. Ignoring entry:" << cfgEntries[enumName]->getFullKey();
            continue;
        }

        defShortcut(actionsEnum.value(i), stringEntry);
    }
}

void ExtActionContainer::defShortcut(int action, CfgStringEntry *cfgEntry)
{
    shortcuts[action] = cfgEntry;

    actionIdMapper->setMapping(cfgEntry, action);
    QObject::connect(cfgEntry, SIGNAL(changed(QVariant)), actionIdMapper, SLOT(map()));
}

void ExtActionContainer::setShortcutContext(const QList<qint32> actions, Qt::ShortcutContext context)
{
    foreach (qint32 act, actions)
        actionMap[act]->setShortcutContext(context);
}

void ExtActionContainer::attachActionInMenu(int parentAction, int childAction, QToolBar* toolbar)
{
    attachActionInMenu(parentAction, actionMap[childAction], toolbar);
}

void ExtActionContainer::attachActionInMenu(int parentAction, QAction* childAction, QToolBar* toolbar)
{
    QToolButton* button = dynamic_cast<QToolButton*>(toolbar->widgetForAction(actionMap[parentAction]));
    QMenu* menu = button->menu();

    if (!menu)
    {
        menu = new QMenu(button);
        button->setMenu(menu);
        button->setPopupMode(QToolButton::MenuButtonPopup);
    }

    menu->addAction(childAction);
}

void ExtActionContainer::updateShortcutTips()
{
}

void ExtActionContainer::createAction(int action, QAction* qAction, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    if (!owner)
        owner = container;
    else
        owner->addAction(qAction);

    qAction->setParent(owner);
    actionMap[action] = qAction;
    QObject::connect(qAction, SIGNAL(triggered()), receiver, slot);
    container->addAction(qAction);
}

void ExtActionContainer::deleteActions()
{
    foreach (QAction* action, actionMap.values())
        delete action;

    actionMap.clear();
}

void ExtActionContainer::refreshShortcuts()
{
    foreach (int action, actionMap.keys())
    {
        if (!shortcuts.contains(action))
            continue;

        if (noConfigShortcutActions.contains(action))
            continue;

        refreshShortcut(action);
    }
}

void ExtActionContainer::refreshShortcut(int action)
{
    actionMap[action]->setShortcut(QKeySequence(shortcuts[action]->get()));
    actionMap[action]->setToolTip(actionMap[action]->text()+QString(" (%1)").arg(shortcuts[action]->get()));
}

QAction* ExtActionContainer::getAction(int action)
{
    if (!actionMap.contains(action))
        return nullptr;

    return actionMap.value(action);
}
