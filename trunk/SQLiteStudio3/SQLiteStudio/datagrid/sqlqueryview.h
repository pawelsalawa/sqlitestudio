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

class SqlQueryView : public QTableView, public ExtActionContainer
{
        Q_OBJECT
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
            SORT_DIALOG
        };

        explicit SqlQueryView(QWidget* parent = 0);
        virtual ~SqlQueryView();
        QList<SqlQueryItem*> getSelectedItems();
        SqlQueryItem* getCurrentItem();
        SqlQueryModel* getModel();
        void setModel(QAbstractItemModel *model);
        SqlQueryItem *itemAt(const QPoint& pos);

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
        bool handleDoubleClick(SqlQueryItem* item);
        QIcon getSortIcon(int columnIndex, QueryExecutor::Sort::Order order);

        SqlQueryItemDelegate* itemDelegate;
        QMenu* contextMenu;
        WidgetCover* widgetCover = nullptr;
        QPushButton* cancelButton = nullptr;
        QProgressBar* busyBar = nullptr;
        QList<QAction*> additionalActions;

    private slots:
        void updateCommitRollbackActions(bool enabled);
        void customContextMenuRequested(const QPoint& pos);
        void openSortDialog();

    public slots:
        void executionStarted();
        void executionEnded();
        void sortingUpdated(const QueryExecutor::SortList& sortOrder);
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
