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
    if (!btn)
        return action;

    btn->setMaximumWidth(400);
    btn->installEventFilter(this);
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

void TaskBar::taskBarMenuAboutToShow()
{
    // This is a hack. We want to display "Ctrl+W" shortcut to the user in this menu, but assigning that shortcut
    // permanently to the action makes it ambigous to Qt, because it's already a standard shortcut,
    // thus making Qt confused and this shortcut working only every second time.
    // Here we assign the shortcut only for the time of displaying the menu. Rest of the time it's not assigned.
    QList<QKeySequence> bindings = QKeySequence::keyBindings(QKeySequence::Close);
    if (bindings.size() > 0)
        MAINWINDOW->getAction(MainWindow::Action::CLOSE_WINDOW)->setShortcut(bindings.first());
}

void TaskBar::taskBarMenuAboutToHide()
{
    MAINWINDOW->getAction(MainWindow::Action::CLOSE_WINDOW)->setShortcut(QKeySequence());
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

void TaskBar::setActiveTask(QAction* task)
{
    if (!task)
        return;

    task->trigger();
}

void TaskBar::initContextMenu(ExtActionContainer* mainWin)
{
    // MainWindow is passed as argument to this function, so it's not referenced with MAINWINDOW macro,
    // because that macro causes MainWindow initialization and this caused endless loop.
    taskMenu = new QMenu(this);
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_WINDOW));
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_ALL_WINDOWS));
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_OTHER_WINDOWS));
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_ALL_WINDOWS_LEFT));
    taskMenu->addAction(mainWin->getAction(MainWindow::CLOSE_ALL_WINDOWS_RIGHT));
    taskMenu->addSeparator();
    taskMenu->addAction(mainWin->getAction(MainWindow::RESTORE_WINDOW));
    taskMenu->addAction(mainWin->getAction(MainWindow::RENAME_WINDOW));

    connect(taskMenu, SIGNAL(aboutToShow()), this, SLOT(taskBarMenuAboutToShow()));
    connect(taskMenu, SIGNAL(aboutToHide()), this, SLOT(taskBarMenuAboutToHide()));
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
        for (QAction* action : tasks)
        {
            btn = getToolButton(action);
            if (btn && btn->x() >= position.x())
                return action;
        }
    }
    else
    {
        for (QAction* action : tasks)
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
    drag->exec(Qt::MoveAction);
    return true;
}

void TaskBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasFormat(mimeDataId))
        return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    dragTaskTo(dragStartTask, event->pos());
#else
    dragTaskTo(dragStartTask, event->position().toPoint());
#endif
    event->acceptProposedAction();
}

void TaskBar::dragMoveEvent(QDragMoveEvent* event)
{
    if (!event->mimeData()->hasFormat(mimeDataId))
        return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    dragTaskTo(dragStartTask, event->pos());
#else
    dragTaskTo(dragStartTask, event->position().toPoint());
#endif
    event->acceptProposedAction();
}

void TaskBar::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();
}

bool TaskBar::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress && dynamic_cast<QMouseEvent*>(event)->button() == Qt::MiddleButton)
    {
        QToolButton* btn = dynamic_cast<QToolButton*>(obj);
        if (btn && btn->defaultAction())
        {
            btn->defaultAction()->trigger();
            MDIAREA->closeActiveSubWindow();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
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

    dragCurrentIndex = positionIndex;

    removeAction(task);

    if (positionIndex >= tasks.size())
    {
        addAction(task);
        tasks.removeOne(task);
        tasks << task;
    }
    else
    {
        int oldIdx = tasks.indexOf(task);

        // If we move from left to right, the positionIndex actually points to 1 position after,
        // so insertAction() can expect its "before action" first argument.
        // Although at this step we want precise position index to move the task on the list,
        // so if this is movement from left to right, we deduct 1 from the index.
        int newTaskIdx = (positionIndex > oldIdx) ? (positionIndex - 1) : positionIndex;
        if (oldIdx == newTaskIdx)
            return;

        insertAction(tasks.at(positionIndex), task);
        tasks.move(oldIdx, newTaskIdx);
    }

    connect(getToolButton(task), SIGNAL(pressed()), this, SLOT(mousePressed()));
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

QAction* TaskBar::getActiveTask() const
{
    QAction* checked = taskGroup.checkedAction();
    if (!checked)
        return nullptr;

    return checked;
}

QAction* TaskBar::getNextTask(QAction* from) const
{
    if (!from)
        from = getActiveTask();

    if (!from)
        return nullptr;

    int idx = tasks.indexOf(from);
    idx++;
    if (idx < tasks.size())
        return tasks[idx];

    return nullptr;
}

QAction* TaskBar::getPrevTask(QAction* from) const
{
    if (!from)
        from = getActiveTask();

    if (!from)
        return nullptr;

    int idx = tasks.indexOf(from);
    idx--;
    if (idx > 0)
        return tasks[idx];

    return nullptr;
}
