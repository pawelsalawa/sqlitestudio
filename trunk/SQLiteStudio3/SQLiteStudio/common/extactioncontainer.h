#ifndef extactionCONTAINER_H
#define extactionCONTAINER_H

#include "iconmanager.h"
#include <QString>
#include <QHash>
#include <QSet>
#include <QKeySequence>
#include <QShortcut>

class QAction;
class QObject;
class QWidget;
class QActionGroup;
class QToolBar;

class ExtActionContainer
{
    public:
        virtual ~ExtActionContainer();

        QAction* getAction(int action);

    protected:
        QHash<int,QAction*> actionMap;
        QHash<int,QString> shortcuts;
        QSet<int> noConfigShortcutActions;

        virtual void createActions() = 0;
        virtual void setupDefShortcuts() = 0;

        void initActions();
        void createAction(int action, const Icon& icon, const QString& text, const QObject* receiver, const char* slot, QWidget* container,
                          QWidget* owner = 0);
        void createAction(int action, const QString& text, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner = 0);
        void defShortcut(int action, int keySequence);
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

        /**
         * This method is pretty much an overloaded version of defShortcut(), except it accepts StandardKey.
         * Since the StandardKey is a C++ enum it was ambigous to be overloaded, therefore the new name for the method was used.
         */
        void defShortcutStdKey(int action, QKeySequence::StandardKey standardKey);

        void updateShortcutTips();

    private:
        void refreshShortcuts();
        void deleteActions();
        void createAction(int action, QAction* qAction, const QObject* receiver, const char* slot, QWidget* container, QWidget* owner);
};


#endif // extactionCONTAINER_H
