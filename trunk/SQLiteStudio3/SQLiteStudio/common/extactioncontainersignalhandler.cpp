#include "extactioncontainersignalhandler.h"
#include "extactioncontainer.h"

ExtActionContainerSignalHandler::ExtActionContainerSignalHandler(ExtActionContainer* actionContainer) :
    QObject(), actionContainer(actionContainer)
{
}

void ExtActionContainerSignalHandler::handleShortcutChange(int action)
{
    actionContainer->refreshShortcut(action);
}
