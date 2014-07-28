#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common/extactioncontainer.h"
#include "db/db.h"
#include "ui_mainwindow.h"
#include "mdiwindow.h"
#include <QMainWindow>
#include <QHash>

class QUiLoader;
class DbTree;
class StatusField;
class EditorWindow;
class MdiArea;
class QActionGroup;
class MdiWindow;
class ViewWindow;
class TableWindow;
class FormManager;
class DdlHistoryWindow;
class FunctionsEditor;
class CollationsEditor;

CFG_KEY_LIST(MainWindow, QObject::tr("Main window"),
     CFG_KEY_ENTRY(OPEN_SQL_EDITOR,   Qt::ALT + Qt::Key_E,         QObject::tr("Open SQL editor"))
     CFG_KEY_ENTRY(PREV_TASK,         Qt::CTRL + Qt::Key_PageUp,   QObject::tr("Previous window"))
     CFG_KEY_ENTRY(NEXT_TASK,         Qt::CTRL + Qt::Key_PageDown, QObject::tr("Next window"))
     CFG_KEY_ENTRY(HIDE_STATUS_FIELD, Qt::Key_Escape,              QObject::tr("Hide status area"))
     CFG_KEY_ENTRY(OPEN_CONFIG,       Qt::Key_F2,                  QObject::tr("Open configuration dialog"))
)

class MainWindow : public QMainWindow, virtual public ExtActionContainer
{
        Q_OBJECT
        Q_ENUMS(Action)

    public:
        enum Action
        {
            MDI_TILE,
            MDI_CASCADE,
            MDI_TILE_HORIZONTAL,
            MDI_TILE_VERTICAL,
            OPEN_SQL_EDITOR,
            NEXT_TASK,
            PREV_TASK,
            HIDE_STATUS_FIELD,
            OPEN_CONFIG,
            OPEN_DDL_HISTORY,
            OPEN_FUNCTION_EDITOR,
            OPEN_COLLATION_EDITOR,
            EXPORT,
            IMPORT
        };

        static MainWindow* getInstance();

        QAction* getAction(Action action);
        MdiArea* getMdiArea() const;
        DbTree* getDbTree() const;
        StatusField* getStatusField() const;
        void restoreSession();
        void setStyle(const QString& styleName);
        FormManager* getFormManager() const;
        bool eventFilter(QObject* obj, QEvent* e);

    protected:
        void closeEvent(QCloseEvent *event);

    private:
        MainWindow();
        ~MainWindow();

        void init();
        void createActions();
        void setupDefShortcuts();
        void initMenuBar();
        void saveSession(MdiWindow* currWindow);
        void restoreWindowSessions(const QList<QVariant>& windowSessions);
        QString currentStyle() const;
        void closeNonSessionWindows();
        DdlHistoryWindow* openDdlHistory();
        FunctionsEditor* openFunctionEditor();
        CollationsEditor* openCollationEditor();

        template <class T>
        T* openMdiWindow();

        static MainWindow* instance;

        Ui::MainWindow *ui;
        DbTree* dbTree;
        StatusField* statusField;
        QMenu* mdiMenu;
        FormManager* formManager;

    public slots:
        EditorWindow* openSqlEditor();

    private slots:
        void cleanUp();
        void openSqlEditorSlot();
        void refreshMdiWindows();
        void hideStatusField();
        void openConfig();
        void openDdlHistorySlot();
        void openFunctionEditorSlot();
        void openCollationEditorSlot();
        void exportAnything();
        void importAnything();
};

template <class T>
T* MainWindow::openMdiWindow()
{
    T* win = nullptr;
    foreach (MdiWindow* mdiWin, ui->mdiArea->getWindows())
    {
        win = dynamic_cast<T*>(mdiWin->getMdiChild());
        if (win)
        {
            ui->mdiArea->setActiveSubWindow(mdiWin);
            return win;
        }
    }

    win = new T(ui->mdiArea);
    if (win->isInvalid())
    {
        delete win;
        return nullptr;
    }

    ui->mdiArea->addSubWindow(win);
    return win;
}


#define MAINWINDOW MainWindow::getInstance()

#endif // MAINWINDOW_H
