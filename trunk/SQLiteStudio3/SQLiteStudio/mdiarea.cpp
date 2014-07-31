#include "mdiarea.h"
#include "mainwindow.h"
#include "iconmanager.h"
#include "mdichild.h"
#include "mdiwindow.h"
#include "taskbar.h"
#include <QMdiSubWindow>
#include <QAction>
#include <QActionGroup>
#include <QDebug>

MdiArea::MdiArea(QWidget *parent) :
    QMdiArea(parent)
{
}

MdiWindow *MdiArea::addSubWindow(MdiChild *mdiChild)
{
    MdiWindow* mdiWin = new MdiWindow(mdiChild, this);
    QMdiArea::addSubWindow(mdiWin);
    mdiWin->show();

    if (taskBar)
    {
        QAction* action = taskBar->addTask(mdiWin->windowIcon(), mdiWin->windowTitle());
        action->setCheckable(true);
        action->setChecked(true);
        actionToWinMap[action] = mdiWin;
        winToActionMap[mdiWin] = action;

        connect(action, &QAction::triggered, this, &MdiArea::taskActivated);
        connect(mdiWin, &QMdiSubWindow::aboutToActivate, this, &MdiArea::windowActivated);
    }

    if (!mdiChild->handleInitialFocus())
        mdiChild->setFocus();

    emit windowListChanged();
    return mdiWin;
}

MdiWindow *MdiArea::getActiveWindow()
{
    return dynamic_cast<MdiWindow*>(activeSubWindow());
}

void MdiArea::setTaskBar(TaskBar* value)
{
    taskBar = value;
}

TaskBar* MdiArea::getTaskBar() const
{
    return taskBar;
}

QAction* MdiArea::getTaskByWindow(MdiWindow* window)
{
    if (winToActionMap.contains(window))
        return winToActionMap[window];

    return nullptr;
}

QList<MdiWindow*> MdiArea::getWindows()
{
    QList<MdiWindow*> windowList;
    foreach(QAction* action, taskBar->getTasks())
        windowList << actionToWinMap[action];

    return windowList;
}

void MdiArea::taskActivated()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    if (!action)
    {
        qWarning() << "MdiArea::taskActivated() slot called by sender that is not QAction.";
        return;
    }

    setActiveSubWindow(actionToWinMap[action]);
}

void MdiArea::windowDestroyed(MdiWindow* window)
{
    if (!taskBar)
        return;

    QAction* action = winToActionMap[window];
    winToActionMap.remove(window);
    actionToWinMap.remove(action);
    taskBar->removeTask(action);
    delete action;

    emit windowListChanged();
}

void MdiArea::windowActivated()
{
    if (!taskBar)
        return;

    MdiWindow* subWin = dynamic_cast<MdiWindow*>(sender());
    if (!subWin)
    {
        qWarning() << "MdiArea::windowActivated() slot called by sender that is not QMdiSubWindow.";
        return;
    }

    QAction* action = winToActionMap[subWin];
    action->setChecked(true);
}

void MdiArea::tileHorizontally()
{
    if (taskBar->isEmpty())
        return;

    QPoint position(0, 0);
    int winCnt = getWindows().count();
    foreach (MdiWindow *window, getWindows())
    {
        if (window->isMaximized())
            window->showNormal();

        QRect rect(0, 0, width() / winCnt, height());
        window->setGeometry(rect);
        window->move(position);
        position.setX(position.x() + window->width());
    }
}

void MdiArea::tileVertically()
{
    if (taskBar->isEmpty())
        return;

    QPoint position(0, 0);
    int winCnt = getWindows().count();
    foreach (MdiWindow *window, getWindows())
    {
        if (window->isMaximized())
            window->showNormal();

        QRect rect(0, 0, width(), height() / winCnt);
        window->setGeometry(rect);
        window->move(position);
        position.setY(position.y() + window->height());
    }
}

void MdiArea::closeAllButActive()
{
    QList<QMdiSubWindow*> allButActive = subWindowList();
    allButActive.removeOne(activeSubWindow());

    foreach (QMdiSubWindow *window, allButActive)
        window->close();
}

MdiWindow* MdiArea::getWindowByChild(MdiChild *child)
{
    if (!child)
        return nullptr;

    foreach (QMdiSubWindow *window, subWindowList())
        if (window->widget() == child)
            return dynamic_cast<MdiWindow*>(window);

    return nullptr;
}

MdiWindow* MdiArea::getCurrentWindow()
{
    QMdiSubWindow* subWin = activeSubWindow();
    return dynamic_cast<MdiWindow*>(subWin);
}

bool MdiArea::isActiveSubWindow(MdiWindow *window)
{
    if (!window)
        return false;

    QMdiSubWindow* subWin = currentSubWindow();
    if (!subWin)
        return false;

    return window == subWin;
}

bool MdiArea::isActiveSubWindow(MdiChild *child)
{
    if (!child)
        return false;

    QMdiSubWindow* subWin = currentSubWindow();
    if (!subWin)
        return false;

    return child == subWin->widget();
}

QStringList MdiArea::getWindowTitles()
{
    QStringList titles;
    foreach (QMdiSubWindow *subWin, subWindowList())
        titles << subWin->windowTitle();

    return titles;
}

MdiWindow *MdiArea::getWindowByTitle(const QString &title)
{
    foreach (QMdiSubWindow *subWin, subWindowList())
    {
        QString t = subWin->windowTitle();
        if (subWin->windowTitle() == title)
        {
            return dynamic_cast<MdiWindow*>(subWin);
        }
    }

    return nullptr;
}
