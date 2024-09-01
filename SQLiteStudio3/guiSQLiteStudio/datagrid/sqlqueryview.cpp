#include "sqlqueryview.h"
#include "sqlqueryitemdelegate.h"
#include "sqlquerymodel.h"
#include "sqlqueryitem.h"
#include "common/widgetcover.h"
#include "tsvserializer.h"
#include "iconmanager.h"
#include "common/unused.h"
#include "common/extaction.h"
#include "multieditor/multieditordialog.h"
#include "uiconfig.h"
#include "dialogs/sortdialog.h"
#include "sqlitestudio.h"
#include "services/functionmanager.h"
#include "services/notifymanager.h"
#include "windows/editorwindow.h"
#include "mainwindow.h"
#include "common/utils_sql.h"
#include "common/mouseshortcut.h"
#include <QPushButton>
#include <QProgressBar>
#include <QGridLayout>
#include <QDebug>
#include <QFocusEvent>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include <QMimeData>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QScrollBar>

CFG_KEYS_DEFINE(SqlQueryView)

SqlQueryView::SqlQueryView(QWidget *parent) :
    QTableView(parent)
{
    init();
}

SqlQueryView::~SqlQueryView()
{
    delete itemDelegate;
}

void SqlQueryView::init()
{
    itemDelegate = new SqlQueryItemDelegate();
    setItemDelegate(itemDelegate);
    setMouseTracking(true);
//    setEditTriggers(QAbstractItemView::AnyKeyPressed);
    setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::EditKeyPressed);

    setContextMenuPolicy(Qt::CustomContextMenu);
    contextMenu = new QMenu(this);
    referencedTablesMenu = new QMenu(tr("Go to referenced row in..."), contextMenu);

    setHorizontalHeader(new Header(this));

    connect(this, &QWidget::customContextMenuRequested, this, &SqlQueryView::customContextMenuRequested);
    connect(CFG_UI.Fonts.DataView, SIGNAL(changed(QVariant)), this, SLOT(updateFont()));
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
    connect(horizontalHeader(), &QHeaderView::sectionResized, this, [this](int section, int, int newSize)
    {
        if (ignoreColumnWidthChanges)
            return;

        getModel()->setDesiredColumnWidth(section, newSize);
    });
    connect(verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(adjustRowToContents(int)));
    MouseShortcut::forWheel(Qt::ControlModifier,
                            this, SLOT(fontSizeChangeRequested(int)),
                            viewport());

    horizontalHeader()->setSortIndicatorShown(false);
    horizontalHeader()->setSectionsClickable(true);
    updateFont();

    setupWidgetCover();
    initActions();
    setupHeaderMenu();
}

void SqlQueryView::setupWidgetCover()
{
    widgetCover = new WidgetCover(this);
    widgetCover->initWithInterruptContainer();
}

void SqlQueryView::createActions()
{
    createAction(COPY, ICONS.ACT_COPY, tr("Copy"), this, SLOT(copy()), this);
    createAction(COPY_WITH_HEADER, ICONS.ACT_COPY, tr("Copy with headers"), this, SLOT(copyWithHeader()), this);
    createAction(COPY_AS, ICONS.ACT_COPY, tr("Copy as..."), this, SLOT(copyAs()), this);
    createAction(PASTE, ICONS.ACT_PASTE, tr("Paste"), this, SLOT(paste()), this);
    createAction(PASTE_AS, ICONS.ACT_PASTE, tr("Paste as..."), this, SLOT(pasteAs()), this);
    createAction(SET_NULL, ICONS.SET_NULL, tr("Set NULL values"), this, SLOT(setNull()), this);
    createAction(ERASE, ICONS.ERASE, tr("Erase values"), this, SLOT(erase()), this);
    createAction(OPEN_VALUE_EDITOR, ICONS.OPEN_VALUE_EDITOR, "", this, SLOT(openValueEditor()), this); // actual label is set dynamically in setupActionsForMenu()
    createAction(COMMIT, ICONS.COMMIT, tr("Commit"), this, SLOT(commit()), this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback"), this, SLOT(rollback()), this);
    createAction(SELECTIVE_COMMIT, ICONS.COMMIT, tr("Commit selected cells"), this, SLOT(selectiveCommit()), this);
    createAction(SELECTIVE_ROLLBACK, ICONS.ROLLBACK, tr("Rollback selected cells"), this, SLOT(selectiveRollback()), this);
    createAction(EDIT_CURRENT, tr("Edit current cell inline"), this, SLOT(editCurrent()), this);
    createAction(GENERATE_SELECT, "SELECT", this, SLOT(generateSelect()), this);
    createAction(GENERATE_INSERT, "INSERT", this, SLOT(generateInsert()), this);
    createAction(GENERATE_UPDATE, "UPDATE", this, SLOT(generateUpdate()), this);
    createAction(GENERATE_DELETE, "DELETE", this, SLOT(generateDelete()), this);
    createAction(SORT_DIALOG, ICONS.SORT_COLUMNS, tr("Define columns to sort by"), this, SLOT(openSortDialog()), this);
    createAction(RESET_SORTING, ICONS.SORT_RESET, tr("Remove custom sorting"), this, SLOT(resetSorting()), this);
    createAction(INSERT_ROW, ICONS.INSERT_ROW, tr("Insert row"), this, SIGNAL(requestForRowInsert()), this);
    createAction(INSERT_MULTIPLE_ROWS, ICONS.INSERT_ROWS, tr("Insert multiple rows"), this, SIGNAL(requestForMultipleRowInsert()), this);
    createAction(DELETE_ROW, ICONS.DELETE_ROW, tr("Delete selected row"), this, SIGNAL(requestForRowDelete()), this);
    createAction(ADJUST_ROWS_SIZE, tr("Adjust height of rows"), this, SLOT(toggleRowsHeightAdjustment(bool)), this);
    actionMap[ADJUST_ROWS_SIZE]->setCheckable(true);
    actionMap[ADJUST_ROWS_SIZE]->setChecked(false);
    actionMap[RESET_SORTING]->setEnabled(false);
    createAction(INCR_FONT_SIZE, tr("Increase font size", "data view"), this, SLOT(incrFontSize()), this);
    createAction(DECR_FONT_SIZE, tr("Decrease font size", "data view"), this, SLOT(decrFontSize()), this);
    createAction(INVERT_SELECTION, ICONS.SELECTION_INVERT, tr("Invert selection", "data view"), this, SLOT(invertSelection()), this);
}

void SqlQueryView::setupDefShortcuts()
{
    setShortcutContext({ROLLBACK, SET_NULL, ERASE, OPEN_VALUE_EDITOR, COMMIT, COPY, COPY_AS,
                       PASTE, PASTE_AS, ADJUST_ROWS_SIZE, INCR_FONT_SIZE, DECR_FONT_SIZE}, Qt::WidgetWithChildrenShortcut);

    BIND_SHORTCUTS(SqlQueryView, Action);
}

void SqlQueryView::setupActionsForMenu(SqlQueryItem* currentItem, const QList<SqlQueryItem*>& selectedItems)
{
    // Selected items count
    int selCount = selectedItems.size();

    // Uncommitted items count
    QList<SqlQueryItem*> uncommittedItems = getModel()->getUncommittedItems();
    int uncommittedCount = uncommittedItems.size();

    // How many of selected items is editable
    int editableSelCount = selCount;
    for (SqlQueryItem* selItem : getSelectedItems())
        if (selItem->getColumn()->editionForbiddenReason.size() > 0)
            editableSelCount--;

    bool currentItemEditable = (currentItem && currentItem->getColumn()->editionForbiddenReason.size() == 0);

    // Uncommitted & selected items count
    int uncommittedSelCount = 0;
    for (SqlQueryItem* item : uncommittedItems)
        if (selectedItems.contains(item))
            uncommittedSelCount++;

    if (uncommittedCount > 0)
        contextMenu->addAction(actionMap[COMMIT]);

    if (uncommittedSelCount > 0)
        contextMenu->addAction(actionMap[SELECTIVE_COMMIT]);

    if (uncommittedCount > 0)
        contextMenu->addAction(actionMap[ROLLBACK]);

    if (uncommittedSelCount > 0)
        contextMenu->addAction(actionMap[SELECTIVE_ROLLBACK]);

    if (uncommittedCount > 0 && selCount > 0)
        contextMenu->addSeparator();

    // Edit/show label for "open in editor" action
    actionMap[OPEN_VALUE_EDITOR]->setText(currentItemEditable ? tr("Edit value in editor") : tr("Show value in a viewer"));

    if (selCount > 0)
    {
        if (editableSelCount > 0)
        {
            contextMenu->addAction(actionMap[ERASE]);
            contextMenu->addAction(actionMap[SET_NULL]);
        }
        contextMenu->addAction(actionMap[OPEN_VALUE_EDITOR]);
        contextMenu->addSeparator();
    }

    if (selCount == 1 && currentItem && selectedItems.first() == currentItem)
        addFkActionsToContextMenu(currentItem);

    if (selCount > 0)
    {
        QMenu* generateQueryMenu = contextMenu->addMenu(ICONS.GENERATE_QUERY, tr("Generate query for selected cells"));
        generateQueryMenu->addAction(actionMap[GENERATE_SELECT]);

        Db* db = getModel()->getDb();
        if (db && db->isValid())
        {
            QList<FunctionManager::ScriptFunction*> functions = FUNCTIONS->getScriptFunctionsForDatabase(db->getName());
            if (functions.size() > 0)
            {
                QStringList fnNames;
                // Offer functions with undefined arguments or at least 1 defined argument
                for (FunctionManager::ScriptFunction* fn : functions)
                    if (fn->undefinedArgs || fn->arguments.size() >= 1)
                        fnNames << fn->name;
                fnNames.sort();
                QMenu* generateSelectFunctionMenu = generateQueryMenu->addMenu("SELECT function(...)");
                for (const QString& name : fnNames)
                    generateSelectFunctionMenu->addAction(name, this, SLOT(generateSelectFunction()));
            }
        }

        if (getModel()->supportsModifyingQueriesInMenu())
        {
            generateQueryMenu->addAction(actionMap[GENERATE_INSERT]);
            generateQueryMenu->addAction(actionMap[GENERATE_UPDATE]);
            generateQueryMenu->addAction(actionMap[GENERATE_DELETE]);
        }

        contextMenu->addSeparator();
        contextMenu->addAction(actionMap[COPY]);
        contextMenu->addAction(actionMap[COPY_WITH_HEADER]);
        //contextMenu->addAction(actionMap[COPY_AS]); // TODO uncomment when implemented
        contextMenu->addAction(actionMap[PASTE]);
        //contextMenu->addAction(actionMap[PASTE_AS]); // TODO uncomment when implemented
    }
    contextMenu->addSeparator();
    contextMenu->addAction(actionMap[INVERT_SELECTION]);
    contextMenu->addAction(actionMap[ADJUST_ROWS_SIZE]);
    if (additionalActions.size() > 0)
    {
        contextMenu->addSeparator();
        for (QAction*& action : additionalActions)
            contextMenu->addAction(action);
    }
}

void SqlQueryView::setupHeaderMenu()
{
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &SqlQueryView::headerContextMenuRequested);
    headerContextMenu = new QMenu(horizontalHeader());
    headerContextMenu->addAction(actionMap[SORT_DIALOG]);
    headerContextMenu->addAction(actionMap[RESET_SORTING]);
}

QList<SqlQueryItem*> SqlQueryView::getSelectedItems()
{
    QList<SqlQueryItem*> items;
    QModelIndexList idxList = selectionModel()->selectedIndexes();
    QModelIndex currIdx = getCurrentIndex();
    if (!idxList.contains(currIdx) && currIdx.isValid())
        idxList << currIdx;

    if (idxList.size() == 0)
        return items;

    std::sort(idxList.begin(), idxList.end());
    const SqlQueryModel* model = dynamic_cast<const SqlQueryModel*>(idxList.first().model());
    for (const QModelIndex& idx : idxList)
        items << model->itemFromIndex(idx);

    return items;
}

SqlQueryItem* SqlQueryView::getCurrentItem()
{
    QModelIndex idx = getCurrentIndex();
    if (!idx.isValid())
        return nullptr;

    return getModel()->itemFromIndex(idx);
}

SqlQueryModel* SqlQueryView::getModel()
{
    return dynamic_cast<SqlQueryModel*>(model());
}

void SqlQueryView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);
    SqlQueryModel* m = getModel();
    connect(widgetCover, SIGNAL(cancelClicked()), m, SLOT(interrupt()));
    connect(m, &SqlQueryModel::commitStatusChanged, this, &SqlQueryView::updateCommitRollbackActions);
    connect(m, &SqlQueryModel::sortingUpdated, this, &SqlQueryView::sortingUpdated);
    connect(m, &SqlQueryModel::executionSuccessful, this, [this]()
    {
        if (actionMap[ADJUST_ROWS_SIZE]->isChecked())
            verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
    });
}

SqlQueryItem* SqlQueryView::itemAt(const QPoint& pos)
{
    return dynamic_cast<SqlQueryItem*>(getModel()->itemFromIndex(indexAt(pos)));
}

QToolBar* SqlQueryView::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
}

void SqlQueryView::addAdditionalAction(QAction* action)
{
    additionalActions << action;
}

QModelIndex SqlQueryView::getCurrentIndex() const
{
    return currentIndex();
}

void SqlQueryView::itemActivated(const QModelIndex& index)
{
    if (simpleBrowserMode)
        return;

    if (!index.isValid())
        return;

    SqlQueryItem* item = getModel()->itemFromIndex(index);
    if (!item)
        return;

    if (!editInEditorIfNecessary(item))
        return;

    edit(getCurrentIndex());
}

void SqlQueryView::generateSelect()
{
    QString sql = getModel()->generateSelectQueryForItems(getSelectedItems());
    MAINWINDOW->openSqlEditor(getModel()->getDb(), sql);
}

void SqlQueryView::generateSelectFunction()
{
    QString function = reinterpret_cast<QAction*>(sender())->text();
    QString sql = getModel()->generateSelectFunctionQueryForItems(function, getSelectedItems());
    MAINWINDOW->openSqlEditor(getModel()->getDb(), sql);
    EditorWindow* win = MAINWINDOW->openSqlEditor(getModel()->getDb(), sql);
    if (!win)
        return;

    static_qstring(tpl, "%1(...)");
    win->getMdiWindow()->rename(tpl.arg(function));
    win->execute();
}

void SqlQueryView::generateInsert()
{
    QString sql = getModel()->generateInsertQueryForItems(getSelectedItems());
    MAINWINDOW->openSqlEditor(getModel()->getDb(), sql);
}

void SqlQueryView::generateUpdate()
{
    QString sql = getModel()->generateUpdateQueryForItems(getSelectedItems());
    MAINWINDOW->openSqlEditor(getModel()->getDb(), sql);
}

void SqlQueryView::generateDelete()
{
    QString sql = getModel()->generateDeleteQueryForItems(getSelectedItems());
    MAINWINDOW->openSqlEditor(getModel()->getDb(), sql);
}

void SqlQueryView::editCurrent()
{
    QModelIndex idx = getCurrentIndex();
    if (idx.isValid())
        edit(idx);
}

void SqlQueryView::toggleRowsHeightAdjustment(bool enabled)
{
    QHeaderView* hdr = verticalHeader();
    if (enabled)
    {
        hdr->resizeSections(QHeaderView::ResizeToContents);
    }
    else
    {
        hdr->setSectionResizeMode(QHeaderView::Interactive);
        hdr->resizeSections(QHeaderView::Interactive);
        int height = hdr->defaultSectionSize();
        int rows = getModel()->rowCount();
        for (int row = 0; row < rows; row++)
            hdr->resizeSection(row, height);
    }
}

void SqlQueryView::adjustRowToContents(int section)
{
    verticalHeader()->setSectionResizeMode(section, QHeaderView::ResizeToContents);
}

void SqlQueryView::fontSizeChangeRequested(int delta)
{
    changeFontSize(delta >= 0 ? 1 : -1);
}

void SqlQueryView::incrFontSize()
{
    changeFontSize(1);
}

void SqlQueryView::decrFontSize()
{
    changeFontSize(-1);
}

void SqlQueryView::invertSelection()
{
    SqlQueryModel* model = getModel();
    int rows = model->rowCount();
    int cols = model->columnCount();
    QItemSelectionModel* selection = selectionModel();
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            selection->select(model->index(r, c), QItemSelectionModel::Toggle);

    if (!selection->isSelected(currentIndex()))
    {
        QModelIndexList idxList = selection->selectedIndexes();
        if (!idxList.isEmpty())
            selection->setCurrentIndex(selection->selectedIndexes().first(), QItemSelectionModel::NoUpdate);
    }
}

bool SqlQueryView::editInEditorIfNecessary(SqlQueryItem* item)
{
    if (item->getColumn()->dataType.getType() == DataType::BLOB)
    {
        openValueEditor(item);
        return false;
    }
    return true;
}

void SqlQueryView::paste(const QList<QList<QVariant> >& data)
{
    if (simpleBrowserMode)
        return;

    QList<SqlQueryItem*> selectedItems = getSelectedItems();
    if (selectedItems.isEmpty())
    {
        notifyWarn(tr("No items selected to paste clipboard contents to."));
        return;
    }

    if (getModel()->isStructureOutOfDate())
    {
        notifyWarn(tr("Cannot paste data. Details: %1").arg(tr("Structure of at least one table used has changed since last data was loaded. Reload the data to proceed.")));
        return;
    }

    QSet<QString> warnedColumns;
    bool warnedRowDeletion = false;
    if (data.size() == 1 && data[0].size() == 1)
    {
        QVariant theValue = data[0][0];
        for (SqlQueryItem* item : selectedItems)
        {
            if (!validatePasting(warnedColumns, warnedRowDeletion, item))
                continue;

            item->setValue(theValue, false);
        }

        return;
    }

    SqlQueryItem* topLeft = selectedItems.first();

    int columnCount = getModel()->columnCount();
    int rowCount = getModel()->rowCount();
    int rowIdx = topLeft->row();
    int colIdx = topLeft->column();

    SqlQueryItem* item = nullptr;

    for (const QList<QVariant>& cells : data)
    {
        // Check if we're out of rows range
        if (rowIdx >= rowCount)
        {
            // No more rows available.
            qDebug() << "Tried to paste more rows than available in the grid.";
            break;
        }

        for (const QVariant& cell : cells)
        {
            // Get current cell
            if (colIdx >= columnCount)
            {
                // No more columns available.
                qDebug() << "Tried to paste more columns than available in the grid.";
                break;
            }
            item = getModel()->itemFromIndex(rowIdx, colIdx++);

            // Set value to the cell, if possible
            if (!validatePasting(warnedColumns, warnedRowDeletion, item))
                continue;

            item->setValue(cell, false);
        }

        // Go to next row, first column
        rowIdx++;
        colIdx = topLeft->column();
    }
}

bool SqlQueryView::validatePasting(QSet<QString>& warnedColumns, bool& warnedRowDeletion, SqlQueryItem* item)
{
    if (item->isDeletedRow())
    {
        if (!warnedRowDeletion)
        {
            warnedRowDeletion = true;
            notifyWarn(tr("Cannot paste to a cell. Details: %1").arg(tr("The row is marked for deletion.")));
        }
        return false;
    }

    if (!item->getColumn()->canEdit())
    {
        QString colName = item->getColumn()->displayName;
        if (!warnedColumns.contains(colName))
        {
            warnedColumns << colName;
            notifyWarn(tr("Cannot paste to column %1. Details: %2").arg(colName, item->getColumn()->getEditionForbiddenReason()));
        }
        return false;
    }

    return true;
}

void SqlQueryView::addFkActionsToContextMenu(SqlQueryItem* currentItem)
{
    QList<SqlQueryModelColumn::ConstraintFk*> fkList = currentItem->getColumn()->getFkConstraints();
    if (fkList.isEmpty())
        return;

    QAction* act;
    if (fkList.size() == 1)
    {
        SqlQueryModelColumn::ConstraintFk* fk = fkList.first();
        act = contextMenu->addAction(tr("Go to referenced row in table '%1'").arg(fk->foreignTable));
        connect(act, &QAction::triggered, [this, fk, currentItem](bool) {
            goToReferencedRow(fk->foreignTable, fk->foreignColumn, currentItem->getValue());
        });
        contextMenu->addSeparator();
        return;
    }

    referencedTablesMenu->clear();
    contextMenu->addMenu(referencedTablesMenu);
    for (SqlQueryModelColumn::ConstraintFk* fk : fkList)
    {
        act = referencedTablesMenu->addAction(tr("table '%1'").arg(fk->foreignTable));
        connect(act, &QAction::triggered, [this, fk, currentItem](bool) {
            goToReferencedRow(fk->foreignTable, fk->foreignColumn, currentItem->getValue());
        });
    }
    contextMenu->addSeparator();
}

void SqlQueryView::goToReferencedRow(const QString& table, const QString& column, const QVariant& value)
{
    Db* db = getModel()->getDb();
    if (!db || !db->isValid())
        return;

    static_qstring(sqlTpl, "SELECT * FROM %1 WHERE %2 = %3");

    QString wrappedTable = wrapObjIfNeeded(table);
    QString wrappedColumn = wrapObjIfNeeded(column);
    QString valueStr = wrapValueIfNeeded(value.toString());
    EditorWindow* win = MAINWINDOW->openSqlEditor(db, sqlTpl.arg(wrappedTable, wrappedColumn, valueStr));
    if (!win)
        return;

    win->getMdiWindow()->rename(tr("Referenced row (%1)").arg(table));
    win->execute();
}

void SqlQueryView::copy(bool withHeader)
{
    if (simpleBrowserMode)
        return;

    QList<SqlQueryItem*> selectedItems = getSelectedItems();
    QList<QList<SqlQueryItem*> > groupedItems = SqlQueryModel::groupItemsByRows(selectedItems);

    if (selectedItems.isEmpty())
        return;

    QVariant itemValue;
    QStringList cells;
    QList<QStringList> rows;

    QPair<QString,QList<QList<QVariant>>> theDataPair;
    QList<QList<QVariant>> theData;
    QList<QVariant> theDataRow;

    // Header
    if (withHeader)
    {
        int leftMostColumn = groupedItems.first().first()->column();
        for (SqlQueryModelColumnPtr& col : getModel()->getColumns().mid(leftMostColumn, groupedItems.first().size()))
        {
            theDataRow << col->displayName;
            cells << col->displayName;
        }

        rows << cells;
        cells.clear();

        theData << theDataRow;
        theDataRow.clear();
    }

    // Data
    for (const QList<SqlQueryItem*>& itemsInRows : groupedItems)
    {
        for (SqlQueryItem* item : itemsInRows)
        {
            itemValue = item->getValue();
            if (itemValue.userType() == QMetaType::Double)
                cells << doubleToString(itemValue);
            else
                cells << itemValue.toString();

            theDataRow << itemValue;
        }

        rows << cells;
        cells.clear();

        theData << theDataRow;
        theDataRow.clear();
    }

    QMimeData* mimeData = new QMimeData();
    QString tsv = TsvSerializer::serialize(rows);
    mimeData->setText(tsv);

    QString md5 = QCryptographicHash::hash(tsv.toUtf8(), QCryptographicHash::Md5);
    theDataPair.first = md5;
    theDataPair.second = theData;

    QByteArray serializedData;
    QDataStream stream(&serializedData, QIODevice::WriteOnly);
    stream << theDataPair;
    mimeData->setData(mimeDataId, serializedData);

    qApp->clipboard()->setMimeData(mimeData);
}

void SqlQueryView::changeFontSize(int factor)
{
    auto f = CFG_UI.Fonts.DataView.get();
    f.setPointSize(f.pointSize() + factor);
    CFG_UI.Fonts.DataView.set(f);
}

bool SqlQueryView::getSimpleBrowserMode() const
{
    return simpleBrowserMode;
}

void SqlQueryView::setSimpleBrowserMode(bool value)
{
    simpleBrowserMode = value;
}

void SqlQueryView::setIgnoreColumnWidthChanges(bool ignore)
{
    ignoreColumnWidthChanges = ignore;
}

QMenu* SqlQueryView::getHeaderContextMenu() const
{
    return headerContextMenu;
}

void SqlQueryView::scrollContentsBy(int dx, int dy)
{
    QTableView::scrollContentsBy(dx, dy);
    emit scrolledBy(dx, dy);
}

void SqlQueryView::updateCommitRollbackActions(bool enabled)
{
    actionMap[COMMIT]->setEnabled(enabled);
    actionMap[ROLLBACK]->setEnabled(enabled);
}

void SqlQueryView::customContextMenuRequested(const QPoint& pos)
{
    if (simpleBrowserMode)
        return;

    SqlQueryItem* currentItem = getCurrentItem();
    QList<SqlQueryItem*> selectedItems = getSelectedItems();

    contextMenu->clear();

    setupActionsForMenu(currentItem, selectedItems);
    emit contextMenuRequested(currentItem, selectedItems);

    if (contextMenu->actions().size() == 0)
        return;

    contextMenu->popup(viewport()->mapToGlobal(pos));
}

void SqlQueryView::headerContextMenuRequested(const QPoint& pos)
{
    if (simpleBrowserMode)
        return;

    headerContextMenu->popup(horizontalHeader()->mapToGlobal(pos));
}

void SqlQueryView::openSortDialog()
{
    QStringList columns;
    for (SqlQueryModelColumnPtr col : getModel()->getColumns())
        columns << col->displayName;

    SortDialog dialog(this);
    dialog.setColumns(columns);
    dialog.setSortOrder(getModel()->getSortOrder());
    if (dialog.exec() != QDialog::Accepted)
        return;

    getModel()->setSortOrder(dialog.getSortOrder());
}

void SqlQueryView::resetSorting()
{
    getModel()->setSortOrder(QueryExecutor::SortList());
}

void SqlQueryView::sortingUpdated(const QueryExecutor::SortList& sortOrder)
{
    actionMap[RESET_SORTING]->setEnabled(sortOrder.size() > 0);
}

void SqlQueryView::updateFont()
{
    QFont f = CFG_UI.Fonts.DataView.get();
    QFontMetrics fm(f);
    verticalHeader()->setDefaultSectionSize(fm.height() + 4);
    if (getModel())
        getModel()->repaintAllItems();
}

void SqlQueryView::executionStarted()
{
    beforeExecutionHorizontalPosition = horizontalScrollBar()->sliderPosition();
    widgetCover->show();
}

void SqlQueryView::executionEnded()
{
    if (beforeExecutionHorizontalPosition > -1)
    {
        horizontalScrollBar()->setSliderPosition(beforeExecutionHorizontalPosition);
        emit scrolledBy(beforeExecutionHorizontalPosition, 0);
    }

    widgetCover->hide();
}

void SqlQueryView::setCurrentRow(int row)
{
    setCurrentIndex(model()->index(row, 0));
}

void SqlQueryView::copy()
{
    copy(false);
}

void SqlQueryView::copyWithHeader()
{
    copy(true);
}

void SqlQueryView::paste()
{
    if (simpleBrowserMode)
        return;

    const QMimeData* mimeData = qApp->clipboard()->mimeData();
    if (mimeData->hasFormat(mimeDataId))
    {
        QString tsv = mimeData->text();
        QString md5 = QCryptographicHash::hash(tsv.toUtf8(), QCryptographicHash::Md5);

        QPair<QString,QList<QList<QVariant>>> theDataPair;
        QByteArray serializedData = mimeData->data(mimeDataId);
        QDataStream stream(&serializedData, QIODevice::ReadOnly);
        stream >> theDataPair;

        if (md5 == theDataPair.first)
        {
            paste(theDataPair.second);
            return;
        }
    }

    QList<QStringList> deserializedRows = TsvSerializer::deserialize(mimeData->text());
    bool trimOnPaste = false;
    bool trimOnPasteAsked = false;
    bool pasteAsNull = false;
    bool pasteAsNullAsked = false;

    QList<QVariant> dataRow;
    QList<QList<QVariant>> dataToPaste;
    for (const QStringList& cells : deserializedRows)
    {
        for (const QString& cell : cells)
        {
#if QT_VERSION >= 0x050A00
            if ((cell.front().isSpace() || cell.back().isSpace()) && !trimOnPasteAsked)
#else
            if ((cell.at(0).isSpace() || cell.at(cell.size() - 1).isSpace()) && !trimOnPasteAsked)
#endif
            {
                QMessageBox::StandardButton trimChoice;
                trimChoice = QMessageBox::question(this, tr("Trim pasted text?"),
                                               tr("The pasted text contains leading or trailing white space. Trim it automatically?"));
                trimOnPasteAsked = true;
                trimOnPaste = (trimChoice == QMessageBox::Yes);
            }

            if (cell=="NULL" && !pasteAsNullAsked)
            {
                QMessageBox::StandardButton nullChoice;
                nullChoice = QMessageBox::question(this, tr("Paste \"NULL\" as null value?"),
                                               tr("The pasted text contains \"NULL\" literals. Do you want to consider them as NULL values?"));
                pasteAsNullAsked = true;
                pasteAsNull = (nullChoice == QMessageBox::Yes);
            }

            if (cell=="NULL" && pasteAsNull)
            {
                dataRow << QVariant();
            }
            else
            {
                dataRow << (trimOnPaste ? cell.trimmed() : cell);
            }
        }

        dataToPaste << dataRow;
        dataRow.clear();
    }

    paste(dataToPaste);
}

void SqlQueryView::copyAs()
{
    // TODO copyAs()
}

void SqlQueryView::pasteAs()
{
    // TODO pasteAs()
}

void SqlQueryView::setNull()
{
    if (simpleBrowserMode)
        return;

    for (SqlQueryItem* selItem : getSelectedItems()) {
        if (selItem->getColumn()->editionForbiddenReason.size() > 0)
            continue;

        selItem->setValue(QVariant(QString()), false);
    }
}

void SqlQueryView::erase()
{
    if (simpleBrowserMode)
        return;

    for (SqlQueryItem* selItem : getSelectedItems()) {
        if (selItem->getColumn()->editionForbiddenReason.size() > 0)
            continue;

        selItem->setValue("", false);
    }
}

void SqlQueryView::commit()
{
    if (simpleBrowserMode)
        return;

    getModel()->commit();
}

void SqlQueryView::rollback()
{
    if (simpleBrowserMode)
        return;

    getModel()->rollback();
}

void SqlQueryView::selectiveCommit()
{
    if (simpleBrowserMode)
        return;

    getModel()->commit(getSelectedItems());
}

void SqlQueryView::selectiveRollback()
{
    if (simpleBrowserMode)
        return;

    getModel()->rollback(getSelectedItems());
}

void SqlQueryView::openValueEditor(SqlQueryItem* item)
{
    if (simpleBrowserMode)
        return;

    if (!item)
    {
        qWarning() << "Tried to open value editor while there's no current item. It should not be called in that case.";
        return;
    }

    SqlQueryModelColumn* column = item->getColumn();

    MultiEditorDialog editor(this);
    if (!column->getFkConstraints().isEmpty())
        editor.enableFk(getModel()->getDb(), column);

    editor.setDataType(column->dataType);
    editor.setWindowTitle(tr("Edit value"));
    editor.setValue(item->getValue());
    editor.setReadOnly(!column->canEdit());

    if (editor.exec() == QDialog::Rejected)
        return;

    item->setValue(editor.getValue());
}

void SqlQueryView::openValueEditor()
{
    SqlQueryItem* currentItem = getCurrentItem();
    openValueEditor(currentItem);
}

TYPE_OF_QHASH qHash(SqlQueryView::Action action)
{
    return static_cast<TYPE_OF_QHASH>(action);
}

SqlQueryView::Header::Header(SqlQueryView* parent) :
    QHeaderView(Qt::Horizontal, parent)
{
}

QSize SqlQueryView::Header::sectionSizeFromContents(int section) const
{
    QSize originalSize = QHeaderView::sectionSizeFromContents(section);
    int colCount = dynamic_cast<SqlQueryView*>(parent())->getModel()->columnCount();
    if (colCount <= 5 || CFG_UI.General.ColumnWidthForName.get())
        return originalSize;

    int wd = minHeaderWidth;
    wd = qMin((wd + wd * 20 / colCount), originalSize.width());
    return QSize(wd, originalSize.height());
}
