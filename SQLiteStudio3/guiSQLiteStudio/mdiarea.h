#ifndef MDIAREA_H
#define MDIAREA_H

#include "mdiwindow.h"
#include "guiSQLiteStudio_global.h"
#include <QMdiArea>
#include <QHash>

class TaskBar;
class QActionGroup;
class MdiChild;

class GUI_API_EXPORT MdiArea : public QMdiArea
{
        Q_OBJECT
    public:
        explicit MdiArea(QWidget *parent = 0);

        MdiWindow* addSubWindow(MdiChild* mdiChild);
        MdiWindow* getActiveWindow();
        MdiWindow* getWindowByTitle(const QString& title);
        MdiWindow* getWindowByChild(MdiChild* child);
        MdiWindow* getCurrentWindow();
        bool isActiveSubWindow(MdiWindow* window);
        bool isActiveSubWindow(MdiChild* child);
        QStringList getWindowTitles();
        void setTaskBar(TaskBar *value);
        TaskBar* getTaskBar() const;
        QAction* getTaskByWindow(MdiWindow* window);
        QList<MdiWindow*> getWindows() const;
        QList<MdiChild*> getMdiChilds() const;
        void enforceTaskSelectionAfterWindowClose(QAction* task);
        void enforceCurrentTaskSelectionAfterWindowClose();

        template<class T>
        QList<T*> getMdiChilds() const;

    private:
        QList<MdiWindow*> getWindowsToTile() const;

        TaskBar* taskBar = nullptr;
        QHash<QAction*,MdiWindow*> actionToWinMap;
        QHash<MdiWindow*,QAction*> winToActionMap;
        QAction* taskToSelectAfterWindowClose = nullptr;

    signals:
        void windowListChanged();
        void sessionValueChanged();

    private slots:
        void taskActivated();
        void windowActivated();

    public slots:
        void windowDestroyed(MdiWindow* window);
        void tileHorizontally();
        void tileVertically();
        void closeAllButActive();
        void closeAllLeftToActive();
        void closeAllRightToActive();
};

template<class T>
QList<T*> MdiArea::getMdiChilds() const
{
    QList<T*> childs;
    T* child = nullptr;
    for (MdiWindow* win : getWindows())
    {
        child = dynamic_cast<T*>(win->getMdiChild());
        if (child)
            childs << child;
    }

    return childs;
}

#define MDIAREA MainWindow::getInstance()->getMdiArea()

#endif // MDIAREA_H
