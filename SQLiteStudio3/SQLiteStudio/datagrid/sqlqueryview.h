#ifndef SQLQUERYVIEW_H
#define SQLQUERYVIEW_H

#include "csvformat.h"
#include "common/extactioncontainer.h"
#include "db/queryexecutor.h"
#include <QTableView>

class SqlQueryItemDelegate;
class SqlQueryItem;
class WidgetCover;
class SqlQueryModel;
class SqlQueryModelColumn;
class QPushButton;
class QProgressBar;
class QMenu;

CFG_KEY_LIST(SqlQueryView, QObject::tr("Data grid view"),
    CFG_KEY_ENTRY(COPY,              Qt::CTRL + Qt::Key_C,              QObject::tr("Copy cell(s) contents to clipboard"))
//    CFG_KEY_ENTRY(COPY_AS,           Qt::CTRL + Qt::SHIFT + Qt::Key_C,  QObject::tr(""))
    CFG_KEY_ENTRY(PASTE,             Qt::CTRL + Qt::Key_V,              QObject::tr("Paste cell(s) contents from clipboard"))
//    CFG_KEY_ENTRY(PASTE_AS,          Qt::CTRL + Qt::SHIFT + Qt::Key_V,  QObject::tr(""))
    CFG_KEY_ENTRY(ERASE,             Qt::ALT + Qt::Key_Backspace,       QObject::tr("Set empty value to selected cell(s)"))
    CFG_KEY_ENTRY(SET_NULL,          Qt::Key_Backspace,                 QObject::tr("Set NULL value to selected cell(s)"))
    CFG_KEY_ENTRY(COMMIT,            Qt::CTRL + Qt::Key_Return,         QObject::tr("Commit changes to cell(s) contents"))
    CFG_KEY_ENTRY(ROLLBACK,          Qt::Key_Escape,                    QObject::tr("Rollback changes to cell(s) contents"))
    CFG_KEY_ENTRY(OPEN_VALUE_EDITOR, Qt::ALT + Qt::Key_Return,          QObject::tr("Open contents of selected cell in a separate editor"))
)

class SqlQueryView : public QTableView, public ExtActionContainer
{
        Q_OBJECT
        Q_ENUMS(Action)

    public:
        enum Action
        {
            COPY,
            COPY_AS,
            PASTE,
            PASTE_AS,
            SET_NULL,
            ERASE,
            ROLLBACK,
            COMMIT,
            SELECTIVE_COMMIT,
            SELECTIVE_ROLLBACK,
            OPEN_VALUE_EDITOR,
            SORT_DIALOG,
            RESET_SORTING
        };

        enum ToolBar
        {
        };

        explicit SqlQueryView(QWidget* parent = 0);
        virtual ~SqlQueryView();
        QList<SqlQueryItem*> getSelectedItems();
        SqlQueryItem* getCurrentItem();
        SqlQueryModel* getModel();
        void setModel(QAbstractItemModel *model);
        SqlQueryItem *itemAt(const QPoint& pos);
        QToolBar* getToolBar(int toolbar) const;
        void addAdditionalAction(QAction* action);

    protected:
        void mouseDoubleClickEvent(QMouseEvent* event);

    private:
        void init();
        void setupWidgetCover();
        void createActions();
        void setupDefShortcuts();
        void refreshShortcuts();
        void setupActionsForMenu(SqlQueryItem* currentItem, const QList<SqlQueryItem*>& selectedItems);
        void setupHeaderMenu();
        bool handleDoubleClick(SqlQueryItem* item);

        SqlQueryItemDelegate* itemDelegate;
        QMenu* contextMenu;
        QMenu* headerContextMenu;
        WidgetCover* widgetCover = nullptr;
        QPushButton* cancelButton = nullptr;
        QProgressBar* busyBar = nullptr;
        QList<QAction*> additionalActions;

    private slots:
        void updateCommitRollbackActions(bool enabled);
        void customContextMenuRequested(const QPoint& pos);
        void headerContextMenuRequested(const QPoint& pos);
        void openSortDialog();
        void resetSorting();
        void sortingUpdated(const QueryExecutor::SortList& sortOrder);

    public slots:
        void executionStarted();
        void executionEnded();
        void setCurrentRow(int row);
        void copy(const CsvFormat& format = CsvFormat::CLIPBOARD);
        void paste(const CsvFormat& format = CsvFormat::CLIPBOARD);
        void copyAs();
        void pasteAs();
        void setNull();
        void erase();
        void commit();
        void rollback();
        void selectiveCommit();
        void selectiveRollback();
        void openValueEditor(SqlQueryItem* item);
        void openValueEditor();

    signals:
        void contextMenuRequested(SqlQueryItem* currentItem, const QList<SqlQueryItem*>& selectedItems);
};

int qHash(SqlQueryView::Action action);

#endif // SQLQUERYVIEW_H
