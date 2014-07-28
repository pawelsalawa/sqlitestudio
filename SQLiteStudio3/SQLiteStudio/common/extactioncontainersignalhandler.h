#ifndef EXTACTIONCONTAINERSIGNALHANDLER_H
#define EXTACTIONCONTAINERSIGNALHANDLER_H

#include <QObject>

class ExtActionContainer;

class ExtActionContainerSignalHandler : public QObject
{
    Q_OBJECT

    public:
        ExtActionContainerSignalHandler(ExtActionContainer* actionContainer);

    private:
        ExtActionContainer* actionContainer = nullptr;

    public slots:
        void handleShortcutChange(int action);

};

#endif // EXTACTIONCONTAINERSIGNALHANDLER_H
