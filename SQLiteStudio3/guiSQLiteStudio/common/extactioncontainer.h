#ifndef extactionCONTAINER_H
#define extactionCONTAINER_H

#include "config_builder.h"
#include "extactionprototype.h"
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
class QMenu;
class Icon;

#define CFG_SHORTCUTS_METANAME "Shortcuts"

#define CFG_KEY_LIST(Type, Title, Entries) \
    _CFG_CATEGORIES_WITH_METANAME(Shortcuts##Type, \
        _CFG_CATEGORY_WITH_TITLE(ShortcutsCategory##Type, Entries, Title), \
        CFG_SHORTCUTS_METANAME\
    )

#define CFG_KEY_ENTRY(Name, KeyStr, Title) CFG_ENTRY(QString, Name, QKeySequence(KeyStr).toString(), Title)

#define CFG_KEYS_DEFINE(Type) CFG_DEFINE_LAZY(Shortcuts##Type)

/**
 * @def Declares access object for defined shortuts.
 *
 * This is the same as CFG_INSTANCE for regular config values.
 * It's optional. It doesn't need to be declared, but if you want to refer
 * to keys as to configuration values, then you will need this.
 */
#define CFG_KEYS_INSTANCE(Type) (*Cfg::getShortcuts##Type##Instance())

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

/**
 * @def Finds shortcut config category instance.
 * Finds CfgCategory containing CfgEntry instances of all shortcuts defined for class \arg Type.
 * For example: GET_SHORTCUTS_CATEGORY(EditorWindow)->getTitle()
 * @return CfgCategory instance of a shortcuts configuration used for specified class.
 */
#define GET_SHORTCUTS_CATEGORY(Type) Cfg::getShortcuts##Type##Instance()->ShortcutsCategory##Type

/**
 * @def Finds shortcut config entry instance.
 * Finds CfgEntry used to store shortcut for enumerated action with \arg ActionName in the class \arg Type.
 * For example: GET_SHORTCUT_ENTRY(EditorWindow, EXEC_QUERY)->get().toString()
 * @return CfgEntry instance of a shortcut config entry.
 */
#define GET_SHORTCUT_ENTRY(Type, ActionName) Cfg::getShortcuts##Type##Instance()->ShortcutsCategory##Type.getEntryByName(#ActionName)

class GUI_API_EXPORT ExtActionContainer
{
    private:
        struct GUI_API_EXPORT ActionDetails
        {
            ActionDetails();
            ActionDetails(ExtActionPrototype* action, int position, bool after);

            ExtActionPrototype* action = nullptr;
            int position = -1;
            bool after = false;
        };

        typedef QList<ActionDetails*> ExtraActions;
        typedef QHash<int,ExtraActions> ToolBarToAction;
        typedef QHash<QString,ToolBarToAction> ClassNameToToolBarAndAction;

    public:
        ExtActionContainer();
        virtual ~ExtActionContainer();

        QAction* getAction(int action);
        virtual const QMetaObject* metaObject() const = 0;

        static void refreshShortcutTranslations();

        template <class T>
        static void insertAction(ExtActionPrototype* action, int toolbar = -1);

        template <class T>
        static void insertActionBefore(ExtActionPrototype* action, int beforeAction, int toolbar = -1);

        template <class T>
        static void insertActionAfter(ExtActionPrototype* action, int afterAction, int toolbar = -1);

        template <class T>
        static void removeAction(ExtActionPrototype* action, int toolbar = -1);

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
        void attachActionInMenu(QAction* parentAction, QAction* childAction, QToolBar* toolbar);
        void addSeparatorInMenu(int parentAction, QToolBar* toolbar);
        void addSeparatorInMenu(QAction* parentAction, QToolBar *toolbar);
        void updateShortcutTips();

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

        void handleActionInsert(int toolbar, ActionDetails* details);
        void handleActionRemoval(int toolbar, ActionDetails* details);

    private:
        typedef QPair<int,ActionDetails*> ToolbarAndProto;

        class KeySequenceFilter : public QObject
        {
            public:
                bool eventFilter(QObject* watched, QEvent* e);
        };

        void refreshShortcuts();
        void refreshShortcut(int action);
        void deleteActions();
        void createAction(int action, QAction* qAction, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner);
        void handleExtraActions();
        QMenu* getMenuForAction(QAction* parentAction, QToolBar *toolbar);

        template <class T>
        static QList<T*> getInstances();

        template <class T>
        static void insertAction(ExtActionPrototype* action, int pos, bool after, int toolbar);

        static ClassNameToToolBarAndAction extraActions;
        static QList<ExtActionContainer*> instances;

        QSignalMapper* actionIdMapper = nullptr;
        QHash<QAction*,ToolbarAndProto> extraActionToToolbarAndProto;
        QHash<ToolbarAndProto,QAction*> toolbarAndProtoToAction;
};

template <class T>
void ExtActionContainer::insertAction(ExtActionPrototype* action, int pos, bool after, int toolbar)
{
    ActionDetails* dets = new ActionDetails(action, pos, after);
    QString clsName = T::staticMetaObject.className();
    extraActions[clsName][toolbar] << dets;
    for (T* instance : getInstances<T>())
        instance->handleActionInsert(toolbar, dets);
}

template <class T>
void ExtActionContainer::insertAction(ExtActionPrototype* action, int toolbar)
{
    insertAction<T>(action, -1, false, toolbar);
}

template <class T>
void ExtActionContainer::insertActionAfter(ExtActionPrototype* action, int afterAction, int toolbar)
{
    insertAction<T>(action, afterAction, true, toolbar);
}

template <class T>
void ExtActionContainer::insertActionBefore(ExtActionPrototype* action, int beforeAction, int toolbar)
{
    insertAction<T>(action, beforeAction, false, toolbar);
}

template <class T>
void ExtActionContainer::removeAction(ExtActionPrototype* action, int toolbar)
{
    QString clsName = T::staticMetaObject.className();
    if (!extraActions.contains(clsName))
        return;

    if (!extraActions[clsName].contains(toolbar))
        return;

    ActionDetails* dets = nullptr;
    for (ActionDetails*& d : extraActions[clsName][toolbar])
    {
        if (d->action == action)
        {
            dets = d;
            break;
        }
    }

    if (!dets)
        return;

    for (T* instance : getInstances<T>())
        instance->handleActionRemoval(toolbar, dets);

    extraActions[clsName][toolbar].removeOne(dets);
    delete dets;
}

template <class T>
QList<T*> ExtActionContainer::getInstances()
{
    QList<T*> typedInstances;
    T* typedInstance = nullptr;
    for (ExtActionContainer*& instance : instances)
    {
        typedInstance = dynamic_cast<T*>(instance);
        if (typedInstance)
            typedInstances << typedInstance;
    }
    return typedInstances;
}

#endif // extactionCONTAINER_H
