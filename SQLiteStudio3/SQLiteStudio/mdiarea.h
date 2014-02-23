#ifndef MDIAREA_H
#define MDIAREA_H

#include <QMdiArea>

class TaskBar;
class QActionGroup;
class MdiChild;
class MdiWindow;

class MdiArea : public QMdiArea
{
        Q_OBJECT
    public:
        explicit MdiArea(QWidget *parent = 0);

        MdiWindow* addSubWindow(MdiChild* mdiChild);
        MdiWindow* getWindowByTitle(const QString& title);
        MdiWindow* getWindowByChild(MdiChild* child);
        MdiWindow* getCurrentWindow();
        bool isActiveSubWindow(MdiWindow* window);
        bool isActiveSubWindow(MdiChild* child);
        QStringList getWindowTitles();
        void setTaskBar(TaskBar *value);
        TaskBar* getTaskBar() const;
        QAction* getTaskByWindow(MdiWindow* window);
        QList<MdiWindow*> getWindows();

    private:
        TaskBar* taskBar = nullptr;
        QHash<QAction*,MdiWindow*> actionToWinMap;
        QHash<MdiWindow*,QAction*> winToActionMap;

    signals:
        void windowListChanged();

    private slots:
        void taskActivated();
        void windowActivated();

    public slots:
        void windowDestroyed(MdiWindow* window);
        void tileHorizontally();
        void tileVertically();
};

#define MDIAREA MainWindow::getInstance()->getMdiArea()

#endif // MDIAREA_H
