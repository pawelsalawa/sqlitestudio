#ifndef extactionCONTAINER_H
#define extactionCONTAINER_H

#include "iconmanager.h"
#include "config_builder.h"
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

    public:
        ExtActionContainer();
        virtual ~ExtActionContainer();

        QAction* getAction(int action);

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

    private:
        void refreshShortcuts();
        void refreshShortcut(int action);
        void deleteActions();
        void createAction(int action, QAction* qAction, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner);

        ExtActionContainerSignalHandler* signalHandler = nullptr;
        QSignalMapper* actionIdMapper = nullptr;
};

#endif // extactionCONTAINER_H
