#ifndef extactionCONTAINER_H
#define extactionCONTAINER_H

#include "iconmanager.h"
#include "config_builder.h"
#include "extactionmanagementnotifier.h"
#include <QString>
#include <QHash>
#include <QSet>
#include <QKeySequence>
#include <QShortcut>
#include <QMetaEnum>

class QAction;
class QObject;
class QWidget;
class QActionGroup;
class QToolBar;
class QSignalMapper;
#define CFG_SHORTCUTS_METANAME "Shortcuts"

#define CFG_KEY_LIST(Type, Title, Entries) \
    _CFG_CATEGORIES_WITH_METANAME(Shortcuts##Type, \
        _CFG_CATEGORY_WITH_TITLE(ShortcutsCategory##Type, Entries, Title), \
        CFG_SHORTCUTS_METANAME\
    )

#define CFG_KEY_ENTRY(Name, KeyStr, Title) CFG_ENTRY(QString, Name, QKeySequence(KeyStr).toString(), Title)

#define CFG_KEYS_DEFINE(Type) CFG_DEFINE_LAZY(Shortcuts##Type)

/**
 * @def Binds shortcuts configuration with actions enumerator.
 * @param Type Shortcuts category type that was passed to CFG_KEY_LIST.
 * @param EnumName Enumerator type which lists actions that you want bind shortcuts to.
 *
 * Names of shortcut entries have to match names of enumerator literals in order to bind shortcuts
 * to proper actions.
 */
#define BIND_SHORTCUTS(Type, EnumName) \
    for (int _enumCounter = 0, _totalEnums = staticMetaObject.enumeratorCount(); _enumCounter < _totalEnums; _enumCounter++) \
    { \
        if (QString::fromLatin1(staticMetaObject.enumerator(_enumCounter).name()) == #EnumName) \
        { \
            bindShortcutsToEnum(Cfg::getShortcuts##Type##Instance()->ShortcutsCategory##Type, staticMetaObject.enumerator(_enumCounter)); \
            break; \
        } \
    }

class ExtActionContainerSignalHandler;

class ExtActionContainer
{
    friend class ExtActionContainerSignalHandler;

    struct ActionDetails
    {
        ActionDetails();
        ActionDetails(int position, bool after, const ExtActionManagementNotifierPtr& notifier);

        int position = -1;
        bool after = false;
        ExtActionManagementNotifierPtr notifier;
    };

    typedef QHash<QAction*,ActionDetails> ActionToPosition;
    typedef QHash<int,ActionToPosition> ToolBarToAction;
    typedef QHash<QString,ToolBarToAction> ClassNameToToolBarAndAction;

    public:
        ExtActionContainer();
        virtual ~ExtActionContainer();

        QAction* getAction(int action);

        template <class T>
        static ExtActionManagementNotifierPtr insertAction(QAction* action, int toolbar = -1);

        template <class T>
        static ExtActionManagementNotifierPtr insertActionBefore(QAction* action, int beforeAction, int toolbar = -1);

        template <class T>
        static ExtActionManagementNotifierPtr insertActionAfter(QAction* action, int afterAction, int toolbar = -1);

        template <class T>
        static void removeAction(QAction* action, int toolbar = -1);

        virtual const QMetaObject* metaObject() const = 0;

    protected:
        QHash<int,QAction*> actionMap;
        QHash<int,CfgStringEntry*> shortcuts;
        QSet<int> noConfigShortcutActions;

        virtual void createActions() = 0;
        virtual void setupDefShortcuts() = 0;

        void initActions();
        void createAction(int action, const Icon& icon, const QString& text, const QObject* receiver, const char* slot, QWidget* container,
                          QWidget* owner = 0);
        void createAction(int action, const QString& text, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner = 0);

        /**
         * @brief Binds config shortcut entries with action enumerator.
         * @param cfgCategory Config category with QString entries that have shortcut definitions.
         * @param actionsEnum Enumerator with actions.
         *
         * Binds shortcuts defined in given config category to actions listed by the enumerator.
         * Binding is done by name, that is name of the config entry (in the category) is matched against enumeration name,
         *
         * You don't normally use this method, but instead use BIND_SHORTCUTS.
         */
        void bindShortcutsToEnum(CfgCategory &cfgCategory, const QMetaEnum& actionsEnum);
        void defShortcut(int action, CfgStringEntry* cfgEntry);
        void setShortcutContext(const QList<qint32> actions, Qt::ShortcutContext context);

        /**
         * @brief attachActionInMenu
         * @param parentAction Action that will have a submenu. Must already exist.
         * @param childAction Action to add to the submenu. Must already exist.
         * @param toolbar Toolbar that parentAction is already added to.
         * Puts childAction into submenu of parentAction.
         */
        void attachActionInMenu(int parentAction, int childAction, QToolBar* toolbar);
        void attachActionInMenu(int parentAction, QAction* childAction, QToolBar* toolbar);
        void updateShortcutTips();
        ExtActionContainerSignalHandler* getExtActionContainerSignalHandler() const;

        /**
         * @brief Tells the toolbar object for given toolbar enum value.
         * @param toolbar Toolbar enum value for specific implementation of MdiChild.
         * @return Toolbar object or null of there's no toolbar for given value, or no toolbar at all.
         *
         * The \p toolbar argument should be enum value from the specific implementation of MdiChild,
         * for example for TableWindow it could be TOOLBAR_GRID_DATA, which refers to grid data tab toolbar.
         *
         * For classes with no toolbar this function will always return null;
         *
         * For classes with only one toolbar this method will always return that toolbar, no matter
         * if the \p toolbar argument was correct.
         *
         * For classes with more than one toolbar this method will return proper toolbar objects only
         * when the \p toolbar argument was correct, otherwise it returns null (assuming correct implementation
         * of this method).
         */
        virtual QToolBar* getToolBar(int toolbar) const = 0;

        void handleActionInsert(QAction* action, int toolbar, const ActionDetails& details);
        void handleActionRemoval(QAction* action, int toolbar, const ActionDetails& details);

    private:
        void refreshShortcuts();
        void refreshShortcut(int action);
        void deleteActions();
        void createAction(int action, QAction* qAction, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner);
        void handleExtraActions();

        template <class T>
        static QList<T*> getInstances();

        template <class T>
        static ExtActionManagementNotifierPtr insertAction(QAction* action, int pos, bool after, int toolbar);

        static ClassNameToToolBarAndAction extraActions;
        static QList<ExtActionContainer*> instances;

        ExtActionContainerSignalHandler* signalHandler = nullptr;
        QSignalMapper* actionIdMapper = nullptr;
};

template <class T>
ExtActionManagementNotifierPtr ExtActionContainer::insertAction(QAction* action, int pos, bool after, int toolbar)
{
    ExtActionManagementNotifierPtr notifier = ExtActionManagementNotifierPtr::create(action);
    ActionDetails dets(pos, after, notifier);
    QString clsName = T::staticMetaObject.className();
    extraActions[clsName][toolbar][action] = dets;
    for (T* instance : getInstances<T>())
        instance->handleActionInsert(action, toolbar, dets);

    return notifier;
}

template <class T>
ExtActionManagementNotifierPtr ExtActionContainer::insertAction(QAction* action, int toolbar)
{
    return insertAction<T>(action, -1, false, toolbar);
}

template <class T>
ExtActionManagementNotifierPtr ExtActionContainer::insertActionAfter(QAction* action, int afterAction, int toolbar)
{
    return insertAction<T>(action, afterAction, true, toolbar);
}

template <class T>
ExtActionManagementNotifierPtr ExtActionContainer::insertActionBefore(QAction* action, int beforeAction, int toolbar)
{
    return insertAction<T>(action, beforeAction, false, toolbar);
}

template <class T>
void ExtActionContainer::removeAction(QAction* action, int toolbar)
{
    QString clsName = T::staticMetaObject.className();
    if (!extraActions.contains(clsName))
        return;

    if (!extraActions[clsName].contains(toolbar))
        return;

    if (!extraActions[clsName][toolbar].contains(action))
        return;

    ActionDetails dets = extraActions[clsName][toolbar][action];
    for (T* instance : getInstances<T>())
        instance->handleActionRemoval(action, toolbar, dets);

    extraActions[clsName][toolbar].remove(action);
}

template <class T>
QList<T*> ExtActionContainer::getInstances()
{
    QList<T*> typedInstances;
    T* typedInstance;
    for (ExtActionContainer* instance : instances)
    {
        typedInstance = dynamic_cast<T*>(instance);
        if (typedInstance)
            typedInstances << typedInstance;
    }
    return typedInstances;
}

#endif // extactionCONTAINER_H
