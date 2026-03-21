#include "extactioncontainer.h"
#include "iconmanager.h"
#include "common/global.h"
#include <QSignalMapper>
#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QDebug>
#include <QEvent>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtSystemDetection>
#else
#include <qsystemdetection.h>
#endif

ExtActionContainer::ClassNameToToolBarAndAction ExtActionContainer::extraActions;
QList<ExtActionContainer*> ExtActionContainer::instances;

ExtActionContainer::ExtActionContainer()
{
    actionIdMapper = new QSignalMapper();

    QObject::connect(actionIdMapper, &QSignalMapper::mappedInt, [=, this](int action) {refreshShortcut(action);});
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
    keySeqFilter.installedIn = dynamic_cast<QObject*>(this);
    createActions();
    setupDefShortcuts();
    refreshShortcuts();
    handleExtraActions();
}

void ExtActionContainer::createAction(int action, const Icon& icon, const QString& text, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    QAction* qAction = new QAction(icon, text);
    createAction(action, qAction, receiver, slot, container, owner);
}

void ExtActionContainer::createAction(int action, const QString& text, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    QAction* qAction = new QAction(text);
    createAction(action, qAction, receiver, slot, container, owner);
}

void ExtActionContainer::createAction(int action, QAction* qAction, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner)
{
    if (!owner)
        owner = container;
    else
        owner->addAction(qAction);

    qAction->setParent(owner);
    actionMap[action] = qAction;
    if (QString(slot).toLower().contains("toggled"))
    {
        qAction->setCheckable(true);
        QObject::connect(qAction, SIGNAL(toggled(bool)), receiver, slot);
    }
    else
        QObject::connect(qAction, SIGNAL(triggered(bool)), receiver, slot);

    container->addAction(qAction);
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

void ExtActionContainer::inheritShortcut(int fromAction, QSet<int> toActions)
{
    inheritShortcutFromTo[fromAction] = toActions;
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
    static_qstring(tooltipTpl, "%1 (%2)");

    QSet<int> toActions = inheritShortcutFromTo[action];

    actionMap[action]->removeEventFilter(&keySeqFilter);
    for (int toAction : toActions)
        actionMap[toAction]->removeEventFilter(&keySeqFilter);

    QKeySequence seq(shortcuts[action]->get());
    QString txt = seq.toString(QKeySequence::NativeText);
    actionMap[action]->setShortcut(seq);
    actionMap[action]->setToolTip(tooltipTpl.arg(actionMap[action]->iconText(), txt));
    actionMap[action]->installEventFilter(&keySeqFilter);

    for (int toAction : toActions)
    {
        actionMap[toAction]->setShortcut(seq);
        actionMap[toAction]->setToolTip(tooltipTpl.arg(actionMap[toAction]->iconText(), txt));
        actionMap[toAction]->installEventFilter(&keySeqFilter);
    }
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

void ExtActionContainer::handleActionInsert(int toolbarIdx, ActionDetails* details)
{
    if (details->position > -1 && !actionMap.contains(details->position))
    {
        qWarning() << "Tried to insert action" << details->action->text() << "before action" << details->position
                   << "which is not present in action container:" << metaObject()->className();
        return;
    }

    QToolBar* toolBar = getToolBar(toolbarIdx);
    if (toolbarIdx > -1 && !toolBar)
    {
        qWarning() << "Tried to insert action" << details->action->text() << ", but toolbar was incorrect: " << toolbarIdx
                   << "or there is no toolbar in action container:" << metaObject()->className();
        return;
    }

    QWidget* thisObj = dynamic_cast<QWidget*>(this);
    if (toolbarIdx == -1 && !thisObj)
    {
        qWarning() << "Tried to insert action" << details->action->text() << ", but toolbar=-1 "
                      "and this is not QObject:" << metaObject()->className();
        return;
    }

    QAction* action = details->action->create();
    ToolbarAndProto toolbarAndProto(toolbarIdx, details);
    extraActionToToolbarAndProto[action] = toolbarAndProto;
    toolbarAndProtoToAction[toolbarAndProto] = action;

    if (toolbarIdx > -1)
    {
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
        toolBar->insertAction(beforeQAction, action);
    }
    else
        thisObj->addAction(action);

    QObject::connect(action, &QAction::triggered, [this, details, toolbarIdx]()
    {
        details->action->emitTriggered(this, toolbarIdx);
    });

    details->action->emitInsertedTo(this, toolbarIdx, action);
}

void ExtActionContainer::handleActionRemoval(int toolbarIdx, ActionDetails* details)
{
    QToolBar* toolBar = getToolBar(toolbarIdx);
    if (toolbarIdx > -1 && !toolBar)
    {
        qWarning() << "Tried to remove action" << details->action->text() << ", but toolbar was incorrect: " << toolbarIdx << "or there is no toolbar in action container:"
                   << metaObject()->className();
        return;
    }

    QWidget* thisObj = dynamic_cast<QWidget*>(this);
    if (toolbarIdx == -1 && !thisObj)
    {
        qWarning() << "Tried to remove action" << details->action->text() << ", but toolbar=-1 "
                      "and this is not QObject:" << metaObject()->className();
        return;
    }

    ToolbarAndProto toolbarAndProto(toolbarIdx, details);
    QAction* action = toolbarAndProtoToAction[toolbarAndProto];

    details->action->emitAboutToRemoveFrom(this, toolbarIdx, action);

    if (toolbarIdx > -1)
        toolBar->removeAction(action);
    else
        thisObj->removeAction(action);

    extraActionToToolbarAndProto.remove(action);
    toolbarAndProtoToAction.remove(toolbarAndProto);

    details->action->emitRemovedFrom(this, toolbarIdx, action);
    delete action;
}

QList<QAction*> ExtActionContainer::getNonToolbarExtraActions() const
{
    QList<QAction*> actions;
    for (ToolbarAndProto toolbarAndProto : toolbarAndProtoToAction.keys())
    {
        if (toolbarAndProto.first == -1)
            actions << toolbarAndProtoToAction[toolbarAndProto];
    }
    return actions;
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

bool ExtActionContainer::KeySequenceFilter::eventFilter(QObject* watched, QEvent* e)
{
    if (e->type() == QEvent::Shortcut)
    {
        qobject_cast<QAction*>(watched)->activate(QAction::Trigger);
        return true;
    }
#ifdef Q_OS_MAC
    // On Mac calling QObject::event(e) from here causes crash if the event comes from the
    // macOS native menubar. Still it should be fine to just return false.
    return false;
#else
    return QObject::event(e);
#endif
}
