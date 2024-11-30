#include "extactioncontainer.h"
#include "iconmanager.h"
#include "common/extaction.h"
#include "common/global.h"
#include <QSignalMapper>
#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QDebug>

ExtActionContainer::ClassNameToToolBarAndAction ExtActionContainer::extraActions;
QList<ExtActionContainer*> ExtActionContainer::instances;

ExtActionContainer::ExtActionContainer()
{
    actionIdMapper = new QSignalMapper();

    QObject::connect(actionIdMapper,
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                     &QSignalMapper::mappedInt,
#else
                     // We need to explicitly cast QSignalMapper::mapped to tell which overloaded version of function we want
                     static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped),
#endif
                     [=, this](int action) {refreshShortcut(action);});
    instances << this;
}

ExtActionContainer::~ExtActionContainer()
{
    deleteActions();
    safe_delete(actionIdMapper);
    instances.removeOne(this);
}

void ExtActionContainer::initActions()
{
    createActions();
    setupDefShortcuts();
    refreshShortcuts();
    handleExtraActions();
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
    for (qint32 act : actions)
        actionMap[act]->setShortcutContext(context);
}

void ExtActionContainer::attachActionInMenu(int parentAction, int childAction, QToolBar* toolbar)
{
    attachActionInMenu(parentAction, actionMap[childAction], toolbar);
}

void ExtActionContainer::attachActionInMenu(int parentAction, QAction* childAction, QToolBar* toolbar)
{
    attachActionInMenu(actionMap[parentAction], childAction, toolbar);
}

void ExtActionContainer::attachActionInMenu(QAction* parentAction, QAction* childAction, QToolBar* toolbar)
{
    QMenu* menu = getMenuForAction(parentAction, toolbar);
    menu->addAction(childAction);
}

void ExtActionContainer::addSeparatorInMenu(int parentAction, QToolBar* toolbar)
{
    addSeparatorInMenu(actionMap[parentAction], toolbar);
}

void ExtActionContainer::addSeparatorInMenu(QAction *parentAction, QToolBar* toolbar)
{
    QMenu* menu = getMenuForAction(parentAction, toolbar);
    menu->addSeparator();
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
    QObject::connect(qAction, SIGNAL(triggered(bool)), receiver, slot);
    container->addAction(qAction);
}

void ExtActionContainer::deleteActions()
{
    for (QAction* action : actionMap.values())
        delete action;

    actionMap.clear();
}

void ExtActionContainer::refreshShortcuts()
{
    for (int action : actionMap.keys())
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
    QKeySequence seq(shortcuts[action]->get());
    QString txt = seq.toString(QKeySequence::NativeText);
    actionMap[action]->setShortcut(seq);
    actionMap[action]->setToolTip(actionMap[action]->text() + QString(" (%1)").arg(txt));
}

QAction* ExtActionContainer::getAction(int action)
{
    if (!actionMap.contains(action))
        return nullptr;

    return actionMap.value(action);
}

void ExtActionContainer::refreshShortcutTranslations()
{
    static const QString metaName = CFG_SHORTCUTS_METANAME;
    for (CfgMain* cfgMain : CfgMain::getInstances())
    {
        if (cfgMain->getMetaName() != metaName)
            continue;

        cfgMain->translateTitle();
    }
}

void ExtActionContainer::handleActionInsert(int toolbar, ActionDetails* details)
{
    if (details->position > -1 && !actionMap.contains(details->position))
    {
        qWarning() << "Tried to insert action" << details->action->text() << "before action" << details->position
                   << "which is not present in action container:" << metaObject()->className();
        return;
    }

    QToolBar* toolBar = getToolBar(toolbar);
    if (!toolBar)
    {
        qWarning() << "Tried to insert action" << details->action->text() << ", but toolbar was incorrect: " << toolbar
                   << "or there is no toolbar in action container:" << metaObject()->className();
        return;
    }

    QAction* beforeQAction = actionMap[details->position];
    if (details->after)
    {
        QList<QAction*> acts = toolBar->actions();
        int idx = acts.indexOf(beforeQAction);
        idx++;
        if (idx > 0 && idx < acts.size())
            beforeQAction = acts[idx];
        else
            beforeQAction = nullptr;
    }

    QAction* action = details->action->create();
    toolBar->insertAction(beforeQAction, action);

    ToolbarAndProto toolbarAndProto(toolbar, details);
    extraActionToToolbarAndProto[action] = toolbarAndProto;
    toolbarAndProtoToAction[toolbarAndProto] = action;

    QObject::connect(action, &QAction::triggered, [this, details, toolbar]()
    {
        details->action->emitTriggered(this, toolbar);
    });

    details->action->emitInsertedTo(this, toolbar, action);
}

void ExtActionContainer::handleActionRemoval(int toolbar, ActionDetails* details)
{
    QToolBar* toolBar = getToolBar(toolbar);
    if (!toolBar)
    {
        qWarning() << "Tried to remove action" << details->action->text() << ", but toolbar was incorrect: " << toolbar << "or there is no toolbar in action container:"
                   << metaObject()->className();
        return;
    }


    ToolbarAndProto toolbarAndProto(toolbar, details);
    QAction* action = toolbarAndProtoToAction[toolbarAndProto];

    details->action->emitAboutToRemoveFrom(this, toolbar, action);

    toolBar->removeAction(action);
    extraActionToToolbarAndProto.remove(action);
    toolbarAndProtoToAction.remove(toolbarAndProto);

    details->action->emitRemovedFrom(this, toolbar, action);
    delete action;
}

void ExtActionContainer::handleExtraActions()
{
    QString clsName = metaObject()->className();
    if (!extraActions.contains(clsName))
        return;

    // For each toolbar
    for (int toolbarId : extraActions[clsName].keys())
    {
        // For each action for this toolbar
        for (ActionDetails* actionDetails : extraActions[clsName][toolbarId])
        {
            // Insert action into toolbar, before action's assigned "before" action
            handleActionInsert(toolbarId, actionDetails);
        }
    }
}

QMenu *ExtActionContainer::getMenuForAction(QAction *parentAction, QToolBar* toolbar)
{
    QToolButton* button = dynamic_cast<QToolButton*>(toolbar->widgetForAction(parentAction));
    QMenu* menu = button->menu();
    if (!menu)
    {
        menu = new QMenu(button);
        button->setMenu(menu);
        button->setPopupMode(QToolButton::MenuButtonPopup);
    }
    return menu;
}

ExtActionContainer::ActionDetails::ActionDetails()
{
}

ExtActionContainer::ActionDetails::ActionDetails(ExtActionPrototype* action, int position, bool after) :
    action(action), position(position), after(after)
{
}
