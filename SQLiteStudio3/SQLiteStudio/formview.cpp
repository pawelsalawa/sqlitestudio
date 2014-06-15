#include "formview.h"
#include "common/unused.h"
#include "datagrid/sqlquerymodel.h"
#include "datagrid/sqlqueryview.h"
#include "widgetresizer.h"
#include "datagrid/sqlqueryitem.h"
#include "uiconfig.h"
#include <QDataWidgetMapper>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>

FormView::FormView(QWidget *parent) :
    QScrollArea(parent)
{
    init();
}

void FormView::init()
{
    setWidgetResizable(true);

    dataMapper = new QDataWidgetMapper(this);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    connect(dataMapper, &QDataWidgetMapper::currentIndexChanged, this, &FormView::currentIndexChanged);

    contents = new QWidget();
    QVBoxLayout *contentsLayout = new QVBoxLayout();
    contentsLayout->setSpacing(spacing);
    contentsLayout->setMargin(margins);
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
        disconnect(model.data(), &SqlQueryModel::loadingEnded, this, &FormView::dataLoaded);
        disconnect(value, &SqlQueryModel::commitStatusChanged, this, &FormView::gridCommitRollbackStatusChanged);
    }

    model = value;
    connect(value, &SqlQueryModel::loadingEnded, this, &FormView::dataLoaded);
    connect(value, &SqlQueryModel::commitStatusChanged, this, &FormView::gridCommitRollbackStatusChanged);
}

void FormView::load()
{
    reloadInternal();
    dataMapper->toFirst();
}

void FormView::reload()
{
    int idx = dataMapper->currentIndex();
    reloadInternal();
    dataMapper->setCurrentIndex(idx);
}

void FormView::reloadInternal()
{
    // Cleanup
    dataMapper->clearMapping();
    foreach (QWidget* widget, widgets)
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
    foreach (SqlQueryModelColumnPtr column, model->getColumns())
        addColumn(i++, column->displayName, column->dataType, (column->editionForbiddenReason.size() > 0));
}

bool FormView::isModified() const
{
    return valueModified;
}

void FormView::addColumn(int colIdx, const QString& name, const DataType& dataType, bool readOnly)
{
    // Group with label
    QString groupLabel = name;
    if (!dataType.toString().isEmpty())
        groupLabel += " (" + dataType.toString() + ")";

    QGroupBox* group = new QGroupBox(groupLabel);
    QFont font = group->font();
    font.setBold(true);
    group->setFont(font);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setSpacing(spacing);
    vbox->setMargin(margins);
    group->setLayout(vbox);

    // MultiEditor
    MultiEditor* multiEditor = new MultiEditor();
    font.setBold(false);
    multiEditor->setFont(font);
    multiEditor->setReadOnly(readOnly);
    dataMapper->addMapping(multiEditor, colIdx, "value");
    vbox->addWidget(multiEditor);
    widgets << group;
    editors << multiEditor;
    contents->layout()->addWidget(group);
    this->readOnly << readOnly;

    connect(multiEditor, SIGNAL(modified()), this, SLOT(editorValueModified()));

    // MultiEditor editors
    multiEditor->setDataType(dataType);

    // Resizer
    WidgetResizer* resizer = new WidgetResizer(Qt::Vertical);
    resizer->setWidget(group);
    resizer->setWidgetMinimumSize(0, minimumFieldHeight);
    widgets << resizer;
    contents->layout()->addWidget(resizer);
}

bool FormView::isCurrentRowModifiedInGrid()
{
    if (!model)
        return false;

    QModelIndex startIdx = model->index(gridView->currentIndex().row(), 0);
    QModelIndex endIdx = model->index(gridView->currentIndex().row(), model->columnCount() - 1);
    return model->findIndexes(startIdx, endIdx, SqlQueryItem::DataRole::UNCOMMITED, true, 1).size() > 0;
}

void FormView::updateDeletedState()
{
    SqlQueryItem* item = model->itemFromIndex(dataMapper->currentIndex(), 0);
    if (!item)
        return;

    bool deleted = item->isDeletedRow();
    int i = 0;
    foreach (MultiEditor* editor, editors)
    {
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
    int column;
    foreach (MultiEditor* editor, editors)
    {
        column = dataMapper->mappedSection(editor);
        if (!editor->isModified())
            continue;

        model->setData(model->index(dataMapper->currentIndex(), column), editor->getValue());
    }
}

void FormView::updateFromGrid()
{
    currentIndexUpdating = true;

    dataMapper->setCurrentIndex(gridView->currentIndex().row());

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
    return dataMapper->currentIndex();
}
