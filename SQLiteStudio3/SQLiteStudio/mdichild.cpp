#include "mdichild.h"
#include "mdiwindow.h"
#include "iconmanager.h"
#include "mainwindow.h"
#include "common/extactioncontainersignalhandler.h"
#include <QDebug>

MdiChild::MdiChild(QWidget* parent) :
    QWidget(parent)
{
}

MdiChild::~MdiChild()
{
}

QVariant MdiChild::getSessionValue()
{
    QVariant value = saveSession();
    QHash<QString, QVariant> hash = value.toHash();
    hash["class"] = QString(metaObject()->className());
    return hash;
}

bool MdiChild::applySessionValue(const QVariant& sessionValue)
{
    bool result = restoreSession(sessionValue);
    return result;
}

MdiWindow* MdiChild::getMdiWindow() const
{
    return mdiWindow;
}

void MdiChild::setMdiWindow(MdiWindow* value)
{
    mdiWindow = value;
    if (mdiWindow)
    {
        mdiWindow->setWindowTitle(getTitleForMdiWindow());
        mdiWindow->setWindowIcon(*getIconNameForMdiWindow());
    }
}

bool MdiChild::isInvalid() const
{
    return invalid;
}

bool MdiChild::restoreSessionNextTime()
{
    return true;
}

void MdiChild::updateWindowTitle()
{
    if (mdiWindow)
    {
        QString newTitle = getTitleForMdiWindow();
        if (mdiWindow->windowTitle() != newTitle)
            mdiWindow->rename(newTitle);
    }
}

bool MdiChild::handleInitialFocus()
{
    return false;
}
