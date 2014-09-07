#include "extactionmanagementnotifier.h"
#include "extactioncontainer.h"
#include <QDebug>

ExtActionManagementNotifier::ExtActionManagementNotifier(QAction* action) :
    QObject(nullptr), action(action)
{
    qDebug() << "create notifier" << this;
}

void ExtActionManagementNotifier::inserted(ExtActionContainer* object, QToolBar* toolbar)
{
    emit actionInserted(object, toolbar, action);
}

void ExtActionManagementNotifier::removed(ExtActionContainer* object, QToolBar* toolbar)
{
    emit actionRemoved(object, toolbar, action);
}
