#include "mdiarea.h"
#include "mainwindow.h"
#include "iconmanager.h"
#include "mdichild.h"
#include "mdiwindow.h"
#include "taskbar.h"
#include "uiconfig.h"
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

    if (taskBar)
    {
        if (taskBar->getTasks().size() == 1 && CFG_UI.General.OpenMaximized.get())
            mdiWin->setWindowState(mdiWin->windowState()|Qt::WindowMaximized);
    }

    connect(mdiChild, SIGNAL(sessionValueChanged()), this, SIGNAL(sessionValueChanged()));

    emit windowListChanged();
    emit sessionValueChanged();
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

QList<MdiWindow*> MdiArea::getWindows() const
{
    QList<MdiWindow*> windowList;
    for (QAction* action : taskBar->getTasks())
        windowList << actionToWinMap[action];

    return windowList;
}

QList<MdiChild*> MdiArea::getMdiChilds() const
{
    QList<MdiChild*> childs;
    for (MdiWindow* win : getWindows())
        childs << win->getMdiChild();

    return childs;
}

void MdiArea::enforceTaskSelectionAfterWindowClose(QAction* task)
{
    taskToSelectAfterWindowClose = task;
}

void MdiArea::enforceCurrentTaskSelectionAfterWindowClose()
{
    enforceTaskSelectionAfterWindowClose(getTaskBar()->getActiveTask());
}

QList<MdiWindow*> MdiArea::getWindowsToTile() const
{
    QList<MdiWindow*> list;
    for (MdiWindow *window : getWindows())
    {
        if (window->isMinimized())
            continue;

        list << window;
    }
    return list;
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
    emit sessionValueChanged();
}

void MdiArea::windowDestroyed(MdiWindow* window)
{
    if (!taskBar)
        return;

    QAction* action = winToActionMap[window];
    QAction* taskToSelect = nullptr;
    if (!MAINWINDOW->isClosingApp())
    {
        taskToSelect = taskToSelectAfterWindowClose;
        taskToSelectAfterWindowClose = nullptr;
        if (!taskToSelect || action == taskToSelect || !taskBar->getTasks().contains(taskToSelect))
            taskToSelect = taskBar->getNextTask(action);

        if (!taskToSelect)
            taskToSelect = taskBar->getPrevTask(action);
    }

    winToActionMap.remove(window);
    actionToWinMap.remove(action);
    taskBar->removeTask(action);
    delete action;

    emit windowListChanged();
    emit sessionValueChanged();

    if (taskToSelect)
        taskBar->setActiveTask(taskToSelect);
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
    emit sessionValueChanged();
}

void MdiArea::tileHorizontally()
{
    if (taskBar->isEmpty())
        return;

    bool gotFocus = false;
    QPoint position(0, 0);
    QList<MdiWindow*> windowsToTile = getWindowsToTile();
    int winCnt = windowsToTile.count();
    for (MdiWindow *window : windowsToTile)
    {
        if (window->isMaximized())
            window->showNormal();

        QRect rect(0, 0, width() / winCnt, height());
        window->setGeometry(rect);
        window->move(position);
        position.setX(position.x() + window->width());

        if (window->hasFocus())
            gotFocus = true;
    }

    if (!gotFocus && windowsToTile.size() > 0)
        windowsToTile.first()->setFocus();
}

void MdiArea::tileVertically()
{
    if (taskBar->isEmpty())
        return;

    bool gotFocus = false;
    QPoint position(0, 0);
    QList<MdiWindow*> windowsToTile = getWindowsToTile();
    int winCnt = windowsToTile.count();
    for (MdiWindow *window : windowsToTile)
    {
        if (window->isMaximized())
            window->showNormal();

        QRect rect(0, 0, width(), height() / winCnt);
        window->setGeometry(rect);
        window->move(position);
        position.setY(position.y() + window->height());

        if (window->hasFocus())
            gotFocus = true;
    }

    if (!gotFocus && windowsToTile.size() > 0)
        windowsToTile.first()->setFocus();
}

void MdiArea::closeAllButActive()
{
    QList<QMdiSubWindow*> allButActive = subWindowList();
    allButActive.removeOne(activeSubWindow());

    for (QMdiSubWindow*& window : allButActive)
        window->close();
}

void MdiArea::closeAllLeftToActive()
{
    QList<QAction*> tasks = taskBar->getTasks();
    QAction* activeTask = taskBar->getActiveTask();
    int activeIdx = tasks.indexOf(activeTask);

    for (QAction*& task : tasks.mid(0, activeIdx))
        actionToWinMap[task]->close();
}

void MdiArea::closeAllRightToActive()
{
    QList<QAction*> tasks = taskBar->getTasks();
    QAction* activeTask = taskBar->getActiveTask();
    int activeIdx = tasks.indexOf(activeTask);

    for (QAction*& task : tasks.mid(activeIdx + 1))
        actionToWinMap[task]->close();
}

MdiWindow* MdiArea::getWindowByChild(MdiChild *child)
{
    if (!child)
        return nullptr;

    for (QMdiSubWindow*& window : subWindowList())
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
    for (QMdiSubWindow *subWin : subWindowList())
        titles << subWin->windowTitle();

    return titles;
}

MdiWindow *MdiArea::getWindowByTitle(const QString &title)
{
    for (QMdiSubWindow *subWin : subWindowList())
    {
        QString t = subWin->windowTitle();
        if (subWin->windowTitle() == title)
        {
            return dynamic_cast<MdiWindow*>(subWin);
        }
    }

    return nullptr;
}
