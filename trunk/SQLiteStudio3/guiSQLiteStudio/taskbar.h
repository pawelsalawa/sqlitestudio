#ifndef TASKBAR_H
#define TASKBAR_H

#include "guiSQLiteStudio_global.h"
#include <QToolBar>
#include <QActionGroup>

class QMimeData;
class QToolButton;
class QRubberBand;
class ExtActionContainer;

// TODO enclose task<->mdiWindow relation inside a task class and make it managed by taskbar, not by mdiarea
class GUI_API_EXPORT TaskBar : public QToolBar
{
        Q_OBJECT
    public:
        TaskBar(const QString& title, QWidget *parent = 0);
        explicit TaskBar(QWidget *parent = 0);

        QAction* addTask(const QIcon& icon, const QString& text);
        void removeTask(QAction* action);
        QList<QAction*> getTasks() const;
        bool isEmpty();
        int count();
        QAction* getActiveTask() const;
        QAction* getNextTask(QAction* from = nullptr) const;
        QAction* getPrevTask(QAction* from = nullptr) const;

    protected:
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void dragEnterEvent(QDragEnterEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
        void dropEvent(QDropEvent* event);
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        void init();
        bool handleMouseMoveEvent(QMouseEvent* event);
        QToolButton* getToolButton(QAction* action);
        QAction* getNextClosestAction(const QPoint& position);
        void dragTaskTo(QAction* task, const QPoint& position);
        void dragTaskTo(QAction* task, int positionIndex);
        QMimeData* generateMimeData();
        int getActiveTaskIdx();

        constexpr static const char* mimeDataId = "application/x-sqlitestudio-taskbar-task";

        /**
         * @brief getDropPositionIndex
         * @param task
         * @param position
         * @return Index of action in actions() that drag should be inserting dropped item just before, or -1 to indicate "at the end".
         */
        int getDropPositionIndex(QAction* task, const QPoint& position);

        QActionGroup taskGroup;
        QList<QAction*> tasks;
        QAction* dragStartTask = nullptr;
        QPoint dragStartPosition;
        int dragStartIndex;
        int dragCurrentIndex;
        QMenu* taskMenu = nullptr;

    public slots:
        void nextTask();
        void prevTask();
        void setActiveTask(QAction* task);
        void initContextMenu(ExtActionContainer *mainWin);

    private slots:
        void taskBarMenuRequested(const QPoint& p);
        void mousePressed();
        void taskBarMenuAboutToShow();
        void taskBarMenuAboutToHide();
};

#endif // TASKBAR_H
