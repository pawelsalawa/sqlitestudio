#include "taskbar.h"
#include "mainwindow.h"
#include <QMouseEvent>
#include <QMimeData>
#include <QDataStream>
#include <QDrag>
#include <QToolButton>
#include <QCursor>
#include <QAction>
#include <QStyle>
#include <QRubberBand>
#include <QApplication>
#include <QDebug>
#include <QMenu>

TaskBar::TaskBar(const QString& title, QWidget *parent) :
    QToolBar(title, parent), taskGroup(this)
{
    init();
}

TaskBar::TaskBar(QWidget* parent) :
    QToolBar(parent), taskGroup(this)
{
    init();
}

QAction* TaskBar::addTask(const QIcon& icon, const QString& text)
{
    // A workaround for QAction button (or QToolBar itself) that takes over (and doesn't propagate) mousePressEvent.
    QAction* action = QToolBar::addAction(icon, text);
    tasks << action;
    QToolButton* btn = getToolButton(action);
    btn->setMaximumWidth(400);
    if (!btn)
        return action;

    taskGroup.addAction(action);
    connect(btn, SIGNAL(pressed()), this, SLOT(mousePressed()));
    return action;
}

void TaskBar::removeTask(QAction* action)
{
    tasks.removeOne(action);
    taskGroup.removeAction(action);
    removeAction(action);
}

QList<QAction*> TaskBar::getTasks() const
{
    return tasks;
}

void TaskBar::init()
{
    setAcceptDrops(true);
}

void TaskBar::mousePressed()
{
    dragStartPosition = mapFromGlobal(QCursor::pos());
    dragStartTask = actionAt(dragStartPosition);
    if (dragStartTask)
        dragStartTask->trigger();
}

int TaskBar::getActiveTaskIdx()
{
    QAction* checked = taskGroup.checkedAction();
    if (!checked)
    {
        // Looks like no tasks yet.
        return -1;
    }

    return tasks.indexOf(checked);
}

void TaskBar::nextTask()
{
    int idx = getActiveTaskIdx() + 1;
    if (tasks.size() <= idx)
        return;

    tasks[idx]->trigger();
}

void TaskBar::prevTask()
{
    int idx = getActiveTaskIdx() - 1;
    if (idx < 0)
        return;

    tasks[idx]->trigger();
}

void TaskBar::initContextMenu(ExtActionContainer* mainWin)
{
    // MainWindow is passed as argument to this function, so it's not referenced with MAINWINDOW macro,
    // because that macro causes MainWindow initialization and this caused endless loop.
    taskMenu = new QMenu(this);
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_WINDOW));
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_OTHER_WINDOWS));
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_ALL_WINDOWS));
    taskMenu->addSeparator();
    taskMenu->addAction(mainWin->getAction(MainWindow::RESTORE_WINDOW));
    taskMenu->addAction(mainWin->getAction(MainWindow::RENAME_WINDOW));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(taskBarMenuRequested(QPoint)));
}

void TaskBar::taskBarMenuRequested(const QPoint &p)
{

    QAction* task = actionAt(p);
    bool taskClicked = (task != nullptr);
    if (taskClicked)
        task->trigger();

    MAINWINDOW->updateWindowActions();
    taskMenu->popup(mapToGlobal(p));
}

QToolButton* TaskBar::getToolButton(QAction* action)
{
    return dynamic_cast<QToolButton*>(widgetForAction(action));
}

QAction* TaskBar::getNextClosestAction(const QPoint& position)
{
    QToolButton* btn = nullptr;
    if (orientation() == Qt::Horizontal)
    {
        foreach (QAction* action, tasks)
        {
            btn = getToolButton(action);
            if (btn && btn->x() >= position.x())
                return action;
        }
    }
    else
    {
        foreach (QAction* action, tasks)
        {
            btn = getToolButton(action);
            if (btn && btn->y() >= position.y())
                return action;
        }
    }
    return nullptr;
}

void TaskBar::mousePressEvent(QMouseEvent* event)
{
    QToolBar::mousePressEvent(event);
    dragStartTask = nullptr;
}

void TaskBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!handleMouseMoveEvent(event))
        QToolBar::mouseMoveEvent(event);
}

bool TaskBar::handleMouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return false;

    if (!dragStartTask)
        return false;

    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return false;

    QDrag *drag = new QDrag(this);
    drag->setMimeData(generateMimeData());

    dragStartIndex = tasks.indexOf(dragStartTask);
    return true;
}

void TaskBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasFormat(mimeDataId))
        return;

    dragTaskTo(dragStartTask, event->pos());
    event->acceptProposedAction();
}

void TaskBar::dragMoveEvent(QDragMoveEvent* event)
{
    if (!event->mimeData()->hasFormat(mimeDataId))
        return;

    dragTaskTo(dragStartTask, event->pos());
    event->acceptProposedAction();
}

void TaskBar::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();
}

void TaskBar::dragTaskTo(QAction* task, const QPoint& position)
{
    int idx = getDropPositionIndex(task, position);
    if (idx < 0)
        return;

    dragTaskTo(task, idx);
}

void TaskBar::dragTaskTo(QAction* task, int positionIndex)
{
    if (positionIndex < 0)
        return;

    removeAction(task);

    if (positionIndex >= tasks.size())
        addAction(task);
    else
        insertAction(tasks.at(positionIndex), task);

    connect(getToolButton(task), SIGNAL(pressed()), this, SLOT(mousePressed()));
    dragCurrentIndex = positionIndex;
}

QMimeData* TaskBar::generateMimeData()
{
    QMimeData *mimeData = new QMimeData();
    mimeData->setData(mimeDataId, QByteArray());
    return mimeData;
}

int TaskBar::getDropPositionIndex(QAction* task, const QPoint& position)
{
    QAction* action = actionAt(position);
    if (!action)
        action = getNextClosestAction(position);

    if (!action)
        return tasks.size(); // We moved completly out of actions range, report last possible position.

    if (action == task)
        return -1;

    int newIdx = tasks.indexOf(action);

    QToolButton* btn = getToolButton(action);
    int actionBeginPos;
    int actionEndPos;
    int newPos;
    if (orientation() == Qt::Horizontal)
    {
        actionBeginPos = btn->x();
        actionEndPos = btn->x() + btn->width();
        newPos = position.x();
    }
    else
    {
        actionBeginPos = btn->y();
        actionEndPos = btn->y() + btn->height();
        newPos = position.y();
    }

    if (dragCurrentIndex <= newIdx)
    {
        // D&D from left to right
        if (newPos >= actionBeginPos)
            return newIdx + 1;
        else
            return newIdx;

    }
    else
    {
        // D&D from right to left
        if (newPos <= actionEndPos)
            return newIdx;
        else
            return newIdx + 1;
    }

    return -1; // This also should never happen. All cases should be covered above. But just in case.
}

bool TaskBar::isEmpty()
{
    return tasks.isEmpty();
}

int TaskBar::count()
{
    return tasks.count();
}
