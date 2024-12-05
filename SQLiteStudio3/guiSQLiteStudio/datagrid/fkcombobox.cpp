#include "fkcombobox.h"
#include "common/unused.h"
#include "datagrid/sqlqueryitem.h"
#include "schemaresolver.h"
#include "services/notifymanager.h"
#include "sqlquerymodel.h"
#include "sqlqueryview.h"
#include "uiconfig.h"
#include <QGuiApplication>
#include <QScreen>
#include <QLineEdit>
#include <QScrollBar>
#include <QCompleter>

FkComboBox::FkComboBox(QWidget* parent, int dropDownViewMinWidth)
    : QComboBox(parent), dropDownViewMinWidth(dropDownViewMinWidth)
{
    init();
}

QString FkComboBox::getSqlForFkEditor(Db* db, SqlQueryModelColumn* columnModel, const QVariant& currentValue)
{
    static_qstring(sql, "SELECT %4, %1 FROM %2%3");
    static_qstring(currValueTpl, "(%1 == %2) AS %3");
    static_qstring(currNullValueTpl, "(%1 IS NULL) AS %2");
    static_qstring(dbColTpl, "%1 AS %2");
    static_qstring(conditionTpl, "%1.%2 = %3.%4");
    static_qstring(conditionPrefixTpl, " WHERE %1");

    QStringList selectedCols;
    QStringList fkConditionTables;
    QStringList fkConditionCols;
    QStringList srcCols;
    SchemaResolver resolver(db);

    QList<SqlQueryModelColumn::ConstraintFk*> fkList = columnModel->getFkConstraints();
    int i = 0;
    QString src;
    QString fullSrcCol;
    QString col;
    QString firstSrcCol;
    QStringList usedNames;
    for (SqlQueryModelColumn::ConstraintFk*& fk : fkList)
    {
        col = wrapObjIfNeeded(fk->foreignColumn);
        src = wrapObjIfNeeded(fk->foreignTable);
        if (i == 0)
        {
            fullSrcCol = src + "." + col;
            firstSrcCol = fullSrcCol;
            selectedCols << dbColTpl.arg(fullSrcCol, wrapObjIfNeeded(columnModel->column));
        }

        if (fkConditionTables.contains(src, Qt::CaseInsensitive))
            continue;

        srcCols = resolver.getTableColumns(src);
        for (QString& srcCol : srcCols)
        {
            if (fk->foreignColumn.compare(srcCol, Qt::CaseInsensitive) == 0)
                continue; // Exclude matching column. We don't want the same column several times.

            fullSrcCol = src + "." + wrapObjIfNeeded(srcCol);
            selectedCols << fullSrcCol;
            usedNames << srcCol;
        }

        fkConditionCols << col;
        fkConditionTables << src;

        i++;
    }

    QStringList conditions;
    QString firstSrc = fkConditionTables.first();
    QString firstCol = fkConditionCols.first();
    for (i = 1; i < fkConditionTables.size(); i++)
    {
        src = fkConditionTables[i];
        col = fkConditionCols[i];
        conditions << conditionTpl.arg(firstSrc, firstCol, src, col);
    }

    QString conditionsStr;
    if (!conditions.isEmpty()) {
        conditionsStr = conditionPrefixTpl.arg(conditions.join(", "));
    }

    // Current value column (will be 1 for row which matches current cell value)
    QString currValueColName = generateUniqueName("curr", usedNames);
    QString currValueExpr = currentValue.isNull() ?
                                currNullValueTpl.arg(firstSrcCol, currValueColName) :
                                currValueTpl.arg(firstSrcCol, valueToSqlLiteral(currentValue), currValueColName);

    return sql.arg(
        selectedCols.join(", "),
        fkConditionTables.join(", "),
        conditionsStr,
        currValueExpr
        );
}

void FkComboBox::init(Db* db, SqlQueryModelColumn* columnModel)
{
    this->columnModel = columnModel;
    comboModel->setDb(db);
}

void FkComboBox::setValue(const QVariant& value)
{
    bool doExecQuery = (sourceValue != value || comboModel->getQuery().isNull());
    sourceValue = value;
    setCurrentText(value.toString());

    if (doExecQuery)
    {
        comboModel->setQuery(getSql());
        if (!comboModel->getQuery().isNull())
            comboModel->executeQuery();
    }
}

QVariant FkComboBox::getValue(bool* manualValueUsed, bool* ok) const
{
    manualValueUsed && (*manualValueUsed = false);
    ok && (*ok = true);
    SqlQueryModel* cbModel = dynamic_cast<SqlQueryModel*>(model());
    if (cbModel->isExecutionInProgress() || !cbModel->isAllDataLoaded())
    {
        ok && (*ok = false);
        return QVariant();
    }

    int idx = currentIndex();
    QModelIndex cbCol0Index = cbModel->index(idx, 0);
    QString cbText = currentText();
    bool customValue = false;

    if (currentIndex() > -1 && !cbModel->itemFromIndex(cbCol0Index))
    {
        // Inserted QStandardItem by QComboBox, meaning custom value (out of dropdown model)
        // With Qt 5.15 (maybe earlier) QComboBox started inserting QStandardItems and setting them as currentIndex.
        // Here we're extracting this inserted value and remembering this is the custom value.
        cbText = cbModel->data(cbCol0Index).toString();
        customValue = true;
    }

    // Regardless if its preselected value or custom value, we need to honor empty=null setting
    if (CFG_UI.General.KeepNullWhenEmptyValue.get() && sourceValue.isNull() && cbText.isEmpty())
    {
        ok && (*ok = false);
        return QVariant();
    }

    // Out of index? So it's custom value. Set it and it's done.
    // If we deal with custom value inserted as item, we also just set it and that's it.
    if (idx < 0 || idx >= cbModel->rowCount() || customValue)
    {
        manualValueUsed && (*manualValueUsed = true);
        return cbText;
    }

    // Otherwise we will have at least 2 columns. 1st column is hidden and is meta-column holding 1/0 (1 for value matching current cell value)
    QList<SqlQueryItem *> row = cbModel->getRow(idx);
    if (row.size() < 2 || !row[1])
    {
        // This happens when inexisting value is confirmed with "Enter" key,
        // and rowCount() is apparently incremented, but items not yet.
        // Very likely this was addressed in recent Qt versions (5.15 or a bit earlier)
        // which resulted in value insertion and the "customValue" flag above in this method.
        qCritical() << "Confirmed FK edition, but there is no combo item in the row for index" << idx << ", CB row count is" << cbModel->rowCount();
        manualValueUsed && (*manualValueUsed = true);
        return cbText;
    }

    return row[1]->getValue();
}

void FkComboBox::init()
{
    setEditable(true);

    // Prepare combo dropdown view.
    comboView = new SqlQueryView();
    comboView->setSimpleBrowserMode(true);
    comboView->setMaximumWidth(QGuiApplication::primaryScreen()->size().width());

    connect(comboView->horizontalHeader(), &QHeaderView::sectionResized, this, [this/*, comboView, model*/](int, int, int)
    {
        // This line is supposed to check if the source SqlQueryModel has already finished loading,
        // before updating combo geometry, but I'm not sure at the moment, why & how would that ever need to be checked.
        // Leaving it here for now, in case some bug appears related to this.
//        if (!model->isAllDataLoaded())
//            return;

        updateComboViewGeometry(false);
    });

    comboModel = new SqlQueryModel(comboView);
    comboModel->setView(comboView);

    // When execution is done, update combo.
    connect(comboModel, SIGNAL(aboutToLoadResults()), this, SLOT(fkDataAboutToLoad()));
    connect(comboModel, SIGNAL(executionSuccessful()), this, SLOT(fkDataReady()));
    connect(comboModel, SIGNAL(executionFailed(QString)), this, SLOT(fkDataFailed(QString)));
    connect(this, SIGNAL(currentTextChanged(QString)), this, SLOT(notifyValueModified()));

    // Setup combo, model, etc.
    setModel(comboModel);
    setView(comboView);
    setModelColumn(1);
    view()->viewport()->installEventFilter(new FkComboShowFilter(this));
    view()->verticalScrollBar()->installEventFilter(new FkComboShowFilter(this));

    comboModel->setHardRowLimit(MAX_ROWS_FOR_FK);
    comboModel->setCellDataLengthLimit(FK_CELL_LENGTH_LIMIT);
    comboModel->setAsyncMode(true);

    comboView->verticalHeader()->setVisible(false);
    comboView->horizontalHeader()->setVisible(true);
    comboView->setSelectionMode(QAbstractItemView::SingleSelection);
    comboView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(completer(), QOverload<const QString &>::of(&QCompleter::highlighted),
        [=, this](const QString &value)
    {
        // #5083 In case of case-sensitive mismatch, we need to sync case, so that next/prev navigation with keybord works correctly.
        setCurrentText(value.left(currentText().length()));
    });
}

void FkComboBox::updateComboViewGeometry(bool initial) const
{
    int wd = getFkViewHeaderWidth(true);
    comboView->setMinimumWidth(qMin(qMax(dropDownViewMinWidth, wd), comboView->maximumWidth()));

    if (initial && wd < comboView->minimumWidth())
    {
        // First time, upon showing up
        int currentSize = comboView->horizontalHeader()->sectionSize(1);
        int gap = comboView->minimumWidth() - wd;
        comboView->horizontalHeader()->resizeSection(1, currentSize + gap);
    }

    QWidget* container = comboView->parentWidget();
    if (container->width() > comboView->minimumWidth())
    {
        container->setMaximumWidth(comboView->minimumWidth());
        container->resize(comboView->minimumWidth(), container->height());
    }
}

void FkComboBox::updateCurrentItemIndex(const QString& value)
{
    QModelIndex startIdx = comboModel->index(0, modelColumn());
    QModelIndex endIdx = comboModel->index(comboModel->rowCount() - 1, modelColumn());
    QModelIndexList idxList = comboModel->findIndexes(startIdx, endIdx, SqlQueryItem::DataRole::VALUE, value.isNull() ? currentText() : value, 1, true);

    if (idxList.size() > 0)
    {
        setCurrentIndex(idxList.first().row());
        view()->selectionModel()->setCurrentIndex(idxList.first(), QItemSelectionModel::SelectCurrent);
    }
}

int FkComboBox::getFkViewHeaderWidth(bool includeScrollBar) const
{
    int wd = comboView->horizontalHeader()->length();
    if (includeScrollBar && comboView->verticalScrollBar()->isVisible())
        wd += comboView->verticalScrollBar()->width();

    return wd;
}

void FkComboBox::fkDataAboutToLoad()
{
    beforeLoadValue = currentText();

    // Not editable combo needs to keep track of initially pre-loading value by using source value
    if (!isEditable() && beforeLoadValue.isNull() && !sourceValue.isNull())
        beforeLoadValue = sourceValue.toString();
}

void FkComboBox::fkDataReady()
{
    disableValueChangeNotifications = true;

    comboView->horizontalHeader()->setSectionHidden(0, true);
    comboView->resizeColumnsToContents();
    comboView->resizeRowsToContents();

    updateComboViewGeometry(true);

    if (comboModel->rowCount() > 0)
    {
        QModelIndex startIdx = comboModel->index(0, modelColumn());
        QModelIndex endIdx = comboModel->index(comboModel->rowCount() - 1, modelColumn());
        QModelIndexList idxList = comboModel->findIndexes(startIdx, endIdx, SqlQueryItem::DataRole::VALUE, beforeLoadValue, 1, true);

        if (idxList.size() > 0)
        {
            setCurrentIndex(idxList.first().row());
        }
        else
        {
            setCurrentIndex(-1);
            setEditText(beforeLoadValue);
        }
    }
    else
    {
        setEditText(beforeLoadValue);
    }

    disableValueChangeNotifications = false;
}

void FkComboBox::fkDataFailed(const QString& errorText)
{
    notifyWarn(tr("Cannot edit this cell. Details: %1").arg(errorText));
}

void FkComboBox::notifyValueModified()
{
    if (disableValueChangeNotifications || !comboModel->isAllDataLoaded())
        return;

    oldValue = currentText();
    disableValueChangeNotifications = true;
    updateCurrentItemIndex();
    disableValueChangeNotifications = false;

    emit valueModified();
}

FkComboBox::FkComboShowFilter::FkComboShowFilter(FkComboBox* parentCombo)
    : QObject(parentCombo)
{
}

bool FkComboBox::FkComboShowFilter::eventFilter(QObject* obj, QEvent* event)
{
    UNUSED(obj);
    if (event->type() == QEvent::Show)
        dynamic_cast<FkComboBox*>(parent())->updateComboViewGeometry(true);

    return false;
}

QString FkComboBox::getSql() const
{
    if (columnModel == nullptr) {
        qWarning() << "Called FkComboBox::getSqlForFkEditor() without column model defined. Tried to setValue() before init()?";
        return QString();
    }

    return getSqlForFkEditor(comboModel->getDb(), columnModel, sourceValue);
}

qlonglong FkComboBox::getRowCountForFkEditor(Db* db, const QString& query, bool* isError)
{
    static_qstring(tpl, "SELECT count(*) FROM (%1)");

    QString sql = tpl.arg(query);
    SqlQueryPtr result = db->exec(sql);
    if (isError)
        *isError = result->isError();

    return result->getSingleCell().toLongLong();
}
