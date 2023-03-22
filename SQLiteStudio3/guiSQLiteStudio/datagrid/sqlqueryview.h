#ifndef SQLQUERYVIEW_H
#define SQLQUERYVIEW_H

#include "csvformat.h"
#include "common/extactioncontainer.h"
#include "db/queryexecutor.h"
#include "guiSQLiteStudio_global.h"
#include <QTableView>
#include <QHeaderView>

class SqlQueryItemDelegate;
class SqlQueryItem;
class WidgetCover;
class SqlQueryModel;
class SqlQueryModelColumn;
class QPushButton;
class QProgressBar;
class QMenu;

CFG_KEY_LIST(SqlQueryView, QObject::tr("Data grid view"),
    CFG_KEY_ENTRY(EDIT_CURRENT,      Qt::Key_F2,                        QObject::tr("Edit current cell inline"))
    CFG_KEY_ENTRY(COPY,              Qt::CTRL + Qt::Key_C,              QObject::tr("Copy cell(s) contents to clipboard"))
    CFG_KEY_ENTRY(COPY_WITH_HEADER,  Qt::CTRL + Qt::SHIFT + Qt::Key_C,  QObject::tr("Copy cell(s) contents together with header to clipboard"))
//    CFG_KEY_ENTRY(COPY_AS,           Qt::CTRL + Qt::ALT + Qt::Key_C,  QObject::tr(""))
    CFG_KEY_ENTRY(PASTE,             Qt::CTRL + Qt::Key_V,              QObject::tr("Paste cell(s) contents from clipboard"))
//    CFG_KEY_ENTRY(PASTE_AS,          Qt::CTRL + Qt::ALT + Qt::Key_V,  QObject::tr(""))
    CFG_KEY_ENTRY(ERASE,             Qt::ALT + Qt::Key_Backspace,       QObject::tr("Set empty value to selected cell(s)"))
    CFG_KEY_ENTRY(SET_NULL,          Qt::Key_Backspace,                 QObject::tr("Set NULL value to selected cell(s)"))
    CFG_KEY_ENTRY(COMMIT,            Qt::CTRL + Qt::Key_Return,         QObject::tr("Commit changes to cell(s) contents"))
    CFG_KEY_ENTRY(ROLLBACK,          QKeySequence::Cancel,              QObject::tr("Rollback changes to cell(s) contents"))
    CFG_KEY_ENTRY(DELETE_ROW,        Qt::Key_Delete,                    QObject::tr("Delete selected data row"))
    CFG_KEY_ENTRY(INSERT_ROW,        Qt::Key_Insert,                    QObject::tr("Insert new data row"))
    CFG_KEY_ENTRY(OPEN_VALUE_EDITOR, Qt::ALT + Qt::Key_Return,          QObject::tr("Open contents of selected cell in a separate editor"))
    CFG_KEY_ENTRY(ADJUST_ROWS_SIZE,  Qt::ALT + Qt::Key_H,               QObject::tr("Toggle the height adjustment of rows"))
    CFG_KEY_ENTRY(INCR_FONT_SIZE,    Qt::CTRL + Qt::Key_Plus,           QObject::tr("Increase font size", "data view"))
    CFG_KEY_ENTRY(DECR_FONT_SIZE,    Qt::CTRL + Qt::Key_Minus,          QObject::tr("Decrease font size", "data view"))
)

class GUI_API_EXPORT SqlQueryView : public QTableView, public ExtActionContainer
{
    Q_OBJECT

    public:
        enum Action
        {
            COPY,
            COPY_WITH_HEADER,
            COPY_AS,
            PASTE,
            PASTE_AS,
            SET_NULL,
            ERASE,
            ROLLBACK,
            COMMIT,
            INSERT_ROW,
            INSERT_MULTIPLE_ROWS,
            DELETE_ROW,
            SELECTIVE_COMMIT,
            SELECTIVE_ROLLBACK,
            EDIT_CURRENT,
            OPEN_VALUE_EDITOR,
            SORT_DIALOG,
            RESET_SORTING,
            GENERATE_SELECT,
            GENERATE_INSERT,
            GENERATE_UPDATE,
            GENERATE_DELETE,
            INVERT_SELECTION,
            ADJUST_ROWS_SIZE,
            INCR_FONT_SIZE,
            DECR_FONT_SIZE
        };
        Q_ENUM(Action)

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
        QModelIndex getCurrentIndex() const;
        bool getSimpleBrowserMode() const;
        void setSimpleBrowserMode(bool value);
        void setIgnoreColumnWidthChanges(bool ignore);
        QMenu* getHeaderContextMenu() const;

    protected:
        void scrollContentsBy(int dx, int dy);

    private:
        class Header : public QHeaderView
        {
            public:
                explicit Header(SqlQueryView* parent);

                QSize sectionSizeFromContents(int section) const;
        };

        void init();
        void setupWidgetCover();
        void createActions();
        void setupDefShortcuts();
        void refreshShortcuts();
        void setupActionsForMenu(SqlQueryItem* currentItem, const QList<SqlQueryItem*>& selectedItems);
        void setupHeaderMenu();
        bool editInEditorIfNecessary(SqlQueryItem* item);
        void paste(const QList<QList<QVariant>>& data);
        bool validatePasting(QSet<QString>& warnedColumns, bool& warnedRowDeletion, SqlQueryItem* item);
        void addFkActionsToContextMenu(SqlQueryItem* currentItem);
        void goToReferencedRow(const QString& table, const QString& column, const QVariant& value);
        void copy(bool withHeaders);
        void changeFontSize(int factor);

        constexpr static const char* mimeDataId = "application/x-sqlitestudio-data-view-data";
        constexpr static const int minHeaderWidth = 15;

        SqlQueryItemDelegate* itemDelegate = nullptr;
        QMenu* contextMenu = nullptr;
        QMenu* headerContextMenu = nullptr;
        QMenu* referencedTablesMenu = nullptr;
        WidgetCover* widgetCover = nullptr;
        QPushButton* cancelButton = nullptr;
        QProgressBar* busyBar = nullptr;
        QList<QAction*> additionalActions;
        bool simpleBrowserMode = false;
        bool ignoreColumnWidthChanges = false;
        int beforeExecutionHorizontalPosition = -1;

    private slots:
        void updateCommitRollbackActions(bool enabled);
        void customContextMenuRequested(const QPoint& pos);
        void headerContextMenuRequested(const QPoint& pos);
        void openSortDialog();
        void resetSorting();
        void sortingUpdated(const QueryExecutor::SortList& sortOrder);
        void updateFont();
        void itemActivated(const QModelIndex& index);
        void generateSelect();
        void generateInsert();
        void generateUpdate();
        void generateDelete();
        void editCurrent();
        void toggleRowsHeightAdjustment(bool enabled);
        void adjustRowToContents(int section);
        void fontSizeChangeRequested(int delta);
        void incrFontSize();
        void decrFontSize();
        void invertSelection();

    public slots:
        void executionStarted();
        void executionEnded();
        void setCurrentRow(int row);
        void copy();
        void copyWithHeader();
        void paste();
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
        void requestForRowInsert();
        void requestForMultipleRowInsert();
        void requestForRowDelete();
        void scrolledBy(int dx, int dy);
};

GUI_API_EXPORT int qHash(SqlQueryView::Action action);

#endif // SQLQUERYVIEW_H
