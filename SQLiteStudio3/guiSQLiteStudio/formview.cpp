#include "formview.h"
#include "common/unused.h"
#include "datagrid/fkcombobox.h"
#include "datagrid/sqlquerymodel.h"
#include "datagrid/sqlqueryview.h"
#include "widgetresizer.h"
#include "datagrid/sqlqueryitem.h"
#include "uiconfig.h"
#include "common/datawidgetmapper.h"
#include "iconmanager.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>

CFG_KEYS_DEFINE(FormView)

FormView::FormView(QWidget *parent) :
    QScrollArea(parent)
{
    init();
}

void FormView::init()
{
    setWidgetResizable(true);
    initActions();

    dataMapper = new DataWidgetMapper(this);
    dataMapper->setSubmitFilter([](QWidget* w) -> bool
    {
        return dynamic_cast<MultiEditor*>(w)->isModified();
    });
    connect(dataMapper, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

    contents = new QWidget();
    QVBoxLayout *contentsLayout = new QVBoxLayout();
    contentsLayout->setSpacing(spacing);
    contentsLayout->setContentsMargins(margins, margins, margins, margins);
    contents->setLayout(contentsLayout);

    connect(CFG_UI.General.DataEditorsOrder, SIGNAL(changed(QVariant)), this, SLOT(reload()));

    setWidget(contents);
}

SqlQueryModel* FormView::getModel() const
{
    return model.data();
}

void FormView::setModel(SqlQueryModel* value)
{
    if (!model.isNull())
    {
        disconnect(model.data(), SIGNAL(loadingEnded(bool)), this, SLOT(dataLoaded(bool)));
        disconnect(value, SIGNAL(commitStatusChanged(bool)), this, SLOT(gridCommitRollbackStatusChanged()));
    }

    model = value;
    connect(value, SIGNAL(loadingEnded(bool)), this, SLOT(dataLoaded(bool)));
    connect(value, SIGNAL(commitStatusChanged(bool)), this, SLOT(gridCommitRollbackStatusChanged()));
}

void FormView::load()
{
    shouldReload = true;
    indexForReload = 0;
}

void FormView::reload()
{
    shouldReload = true;
    indexForReload = dataMapper->getCurrentIndex();
    reloadInternal();
}

void FormView::focusFirstEditor()
{
    if (editors.size() == 0)
        return;

    editors.first()->focusThisEditor();
}

void FormView::reloadInternal()
{
    if (!shouldReload)
        return;

    shouldReload = false;

    // Cleanup
    dataMapper->clearMapping();
    for (QWidget*& widget : widgets)
    {
        contents->layout()->removeWidget(widget);
        delete widget;
    }
    widgets.clear();
    editors.clear();
    readOnly.clear();

    // Recreate
    dataMapper->setModel(model.data());
    int i = 0;
    for (SqlQueryModelColumnPtr& column : model->getColumns())
        addColumn(i++, column.data());
}

bool FormView::isModified() const
{
    return valueModified;
}

MultiEditor* FormView::addColumn(int colIdx, SqlQueryModelColumn* column)
{
    bool readOnly = (column->editionForbiddenReason.size() > 0);

    // Group with label
    QString groupLabel = column->displayName;
    if (!column->dataType.toString().isEmpty())
        groupLabel += " (" + column->dataType.toString() + ")";

    // MultiEditor
    MultiEditor* multiEditor = new MultiEditor();
    multiEditor->setReadOnly(readOnly);
    multiEditor->setCornerLabel(groupLabel);
    dataMapper->addMapping(multiEditor, colIdx, "value");
    widgets << multiEditor;
    editors << multiEditor;
    contents->layout()->addWidget(multiEditor);
    this->readOnly << readOnly;

    connect(multiEditor, SIGNAL(modified()), this, SLOT(editorValueModified()));

    // MultiEditor editors
    if (!column->getFkConstraints().isEmpty())
    {
        Db* db = model->getDb();
        QString sql = FkComboBox::getSqlForFkEditor(db, column, QVariant());
        bool countingError = false;
        qlonglong rowCount = FkComboBox::getRowCountForFkEditor(db, sql, &countingError);
        if (!countingError && rowCount <= FkComboBox::MAX_ROWS_FOR_FK)
            multiEditor->enableFk(db, column);
        else
        {
            qDebug() << "FkCombo excluded from FormView for column" << column->column << "due to"
                     << (countingError ?
                             "error with row counting query" :
                             "too many rows in the FK table: " + QString::number(rowCount));
        }
    }

    multiEditor->setDataType(column->dataType);

    // Resizer
    WidgetResizer* resizer = new WidgetResizer(Qt::Vertical);
    resizer->setWidget(multiEditor);
    resizer->setWidgetMinimumSize(0, minimumFieldHeight);
    widgets << resizer;
    contents->layout()->addWidget(resizer);

    return multiEditor;
}

bool FormView::isCurrentRowModifiedInGrid()
{
    if (!model)
        return false;

    QModelIndex startIdx = model->index(gridView->getCurrentIndex().row(), 0);
    QModelIndex endIdx = model->index(gridView->getCurrentIndex().row(), model->columnCount() - 1);
    return model->findIndexes(startIdx, endIdx, SqlQueryItem::DataRole::UNCOMMITTED, true, 1).size() > 0;
}

void FormView::updateDeletedState()
{
    SqlQueryItem* item = model->itemFromIndex(dataMapper->getCurrentIndex(), 0);
    if (!item)
    {
        for (MultiEditor* editor : editors)
            editor->setEnabled(false);

        return;
    }

    bool deleted = item->isDeletedRow();
    int i = 0;
    for (MultiEditor* editor : editors)
    {
        editor->setEnabled(true);
        editor->setDeletedRow(deleted);
        editor->setReadOnly(readOnly[i++] || deleted);
    }
}

void FormView::dataLoaded(bool successful)
{
    if (successful)
        load();
}

void FormView::currentIndexChanged(int index)
{
    valueModified = false;
    emit commitStatusChanged();

    if (gridView.isNull())
        return;

    if (currentIndexUpdating)
        return;

    currentIndexUpdating = true;
    gridView->setCurrentRow(index);
    currentIndexUpdating = false;

    // If row was deleted, we need to make fields readonly
    updateDeletedState();

    emit currentRowChanged();
}

void FormView::editorValueModified()
{
    valueModified = true;
    emit commitStatusChanged();
}

void FormView::gridCommitRollbackStatusChanged()
{
    valueModified = isCurrentRowModifiedInGrid();
    emit commitStatusChanged();
}

void FormView::copyDataToGrid()
{
    dataMapper->submit();
}

void FormView::updateFromGrid()
{
    currentIndexUpdating = true;

    dataMapper->setCurrentIndex(gridView->getCurrentIndex().row());

    // Already modified in grid?
    valueModified = isCurrentRowModifiedInGrid();

    currentIndexUpdating = false;

    updateDeletedState();

    emit currentRowChanged();
}

SqlQueryView* FormView::getGridView() const
{
    return gridView.data();
}

void FormView::setGridView(SqlQueryView* value)
{
    gridView = value;
}

int FormView::getCurrentRow()
{
    return dataMapper->getCurrentIndex();
}

void FormView::createActions()
{
    createAction(COMMIT, ICONS.COMMIT, tr("Commit row", "form view"), this, SIGNAL(requestForCommit()), this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback row", "form view"), this, SIGNAL(requestForRollback()), this);
    createAction(FIRST_ROW, ICONS.PAGE_FIRST, tr("First row", "form view"), this, SIGNAL(requestForFirstRow()), this);
    createAction(PREV_ROW, ICONS.PAGE_PREV, tr("Previous row", "form view"), this, SIGNAL(requestForPrevRow()), this);
    createAction(NEXT_ROW, ICONS.PAGE_NEXT, tr("Next row", "form view"), this, SIGNAL(requestForNextRow()), this);
    createAction(LAST_ROW, ICONS.PAGE_LAST, tr("Last row", "form view"), this, SIGNAL(requestForLastRow()), this);
    createAction(INSERT_ROW, ICONS.INSERT_ROW, tr("Insert new row", "form view"), this, SIGNAL(requestForRowInsert()), this);
    createAction(DELETE_ROW, ICONS.DELETE_ROW, tr("Delete current row", "form view"), this, SIGNAL(requestForRowDelete()), this);
}

void FormView::setupDefShortcuts()
{
    setShortcutContext({ROLLBACK, COMMIT, NEXT_ROW, PREV_ROW, FIRST_ROW, LAST_ROW, INSERT_ROW, DELETE_ROW}, Qt::ApplicationShortcut);

    BIND_SHORTCUTS(FormView, Action);
}

QToolBar* FormView::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return nullptr;
}

void FormView::showEvent(QShowEvent* event)
{
    UNUSED(event);
    reloadInternal();
}
