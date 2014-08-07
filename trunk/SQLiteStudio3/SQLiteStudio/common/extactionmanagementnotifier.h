#ifndef EXTACTIONMANAGEMENTNOTIFIER_H
#define EXTACTIONMANAGEMENTNOTIFIER_H

#include <QObject>
#include <QSharedPointer>

class QToolBar;
class QAction;
class ExtActionContainer;

class ExtActionManagementNotifier : public QObject
{
        Q_OBJECT
    public:
        explicit ExtActionManagementNotifier(QAction* action);

        void inserted(ExtActionContainer* object, QToolBar* toolbar);
        void removed(ExtActionContainer* object, QToolBar* toolbar);

    private:
        QAction* action = nullptr;

    signals:
        void actionInserted(ExtActionContainer* object, QToolBar* toolbar, QAction* action);
        void actionRemoved(ExtActionContainer* object, QToolBar* toolbar, QAction* action);
};

typedef QSharedPointer<ExtActionManagementNotifier> ExtActionManagementNotifierPtr;

#endif // EXTACTIONMANAGEMENTNOTIFIER_H
