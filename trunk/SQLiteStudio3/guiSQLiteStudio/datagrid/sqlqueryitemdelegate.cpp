#include "sqlqueryitemdelegate.h"
#include "sqlquerymodel.h"
#include "sqlqueryitem.h"
#include "common/unused.h"
#include "services/notifymanager.h"
#include "sqlqueryview.h"
#include "uiconfig.h"
#include "common/utils_sql.h"
#include "schemaresolver.h"
#include <QHeaderView>
#include <QPainter>
#include <QEvent>
#include <QLineEdit>
#include <QDebug>
#include <QComboBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QScrollBar>

SqlQueryItemDelegate::SqlQueryItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void SqlQueryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    SqlQueryItem* item = getItem(index);

    if (item->isUncommited())
    {
        painter->setPen(item->isCommitingError() ? CFG_UI.Colors.DataUncommitedError.get() : CFG_UI.Colors.DataUncommited.get());
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
    }
}

QWidget* SqlQueryItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    UNUSED(option);
    if (!index.isValid())
        return nullptr;

    const SqlQueryModel* model = dynamic_cast<const SqlQueryModel*>(index.model());
    SqlQueryItem* item = model->itemFromIndex(index);

    if (item->isDeletedRow())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %1").arg(tr("The row is marked for deletion.")));
        return nullptr;
    }

    if (!item->getColumn()->canEdit())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %1").arg(item->getColumn()->getEditionForbiddenReason()));
        return nullptr;
    }

    if (item->isLimitedValue() &&!item->loadFullData().isNull() && model->isStructureOutOfDate())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %1").arg(tr("Structure of this table has changed since last data was loaded. Reload the data to proceed.")));
        return nullptr;
    }

    if (!item->getColumn()->getFkConstraints().isEmpty())
        return getFkEditor(item, parent, model);

    return getEditor(item->getValue().userType(), parent);
}

QString SqlQueryItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    UNUSED(locale);

    if (value.type() == QVariant::Double)
        return doubleToString(value.toDouble());

    return QStyledItemDelegate::displayText(value, locale);
}

void SqlQueryItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* cb = dynamic_cast<QComboBox*>(editor);
    QLineEdit* le = dynamic_cast<QLineEdit*>(editor);
    if (cb) {
        setEditorDataForFk(cb, index);
    } else if (le) {
        setEditorDataForLineEdit(le, index);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void SqlQueryItemDelegate::setEditorDataForFk(QComboBox* cb, const QModelIndex& index) const
{
    const SqlQueryModel* queryModel = dynamic_cast<const SqlQueryModel*>(index.model());
    SqlQueryItem* item = queryModel->itemFromIndex(index);
    QVariant modelData = item->getValue();
    int idx = cb->findData(modelData, Qt::UserRole);
    if (idx == -1 )
    {
        cb->addItem(modelData.toString(), modelData);
        idx = cb->count() - 1;

        SqlQueryView* view = dynamic_cast<SqlQueryView*>(cb->view());
        view->resizeColumnsToContents();
        view->setMinimumWidth(view->horizontalHeader()->length());
    }
    cb->setCurrentIndex(idx);
}

void SqlQueryItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* cb = dynamic_cast<QComboBox*>(editor);
    QLineEdit* le = dynamic_cast<QLineEdit*>(editor);
    if (cb) {
        setModelDataForFk(cb, model, index);
    } else if (le) {
        setModelDataForLineEdit(le, model, index);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }

    SqlQueryModel* queryModel = const_cast<SqlQueryModel*>(dynamic_cast<const SqlQueryModel*>(index.model()));
    queryModel->notifyItemEditionEnded(index);
}

void SqlQueryItemDelegate::setModelDataForFk(QComboBox* cb, QAbstractItemModel* model, const QModelIndex& index) const
{
    SqlQueryModel* cbModel = dynamic_cast<SqlQueryModel*>(cb->model());
    if (cbModel->isExecutionInProgress() || !cbModel->isAllDataLoaded())
        return;

    QString cbText = cb->currentText();
    if (CFG_UI.General.KeepNullWhenEmptyValue.get() && model->data(index, Qt::EditRole).isNull() && cbText.isEmpty())
        return;

    int idx = cb->currentIndex();
    if (idx < 0 || idx >= cbModel->rowCount())
    {
        model->setData(index, cbText, Qt::EditRole);
        return;
    }

    QList<SqlQueryItem *> row = cbModel->getRow(idx);
    if (!row[0])
    {
        // This happens when inexisting value is confirmed with "Enter" key,
        // cause rowCount() is apparently incremented, but items not yet.
        model->setData(index, cbText, Qt::EditRole);
        return;
    }

    QVariant comboData = row[0]->getValue();
    if (cbText != comboData.toString())
        comboData = cbText;

    model->setData(index, comboData, Qt::EditRole);
}

void SqlQueryItemDelegate::setModelDataForLineEdit(QLineEdit* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QString value = editor->text();

    if (CFG_UI.General.KeepNullWhenEmptyValue.get() && model->data(index, Qt::EditRole).isNull() && value.isEmpty())
        return;

    bool ok;
    QVariant variant = value.toLongLong(&ok);
    if (ok)
    {
        model->setData(index, variant, Qt::EditRole);
        return;
    }

    variant = value.toDouble(&ok);
    if (ok)
    {
        model->setData(index, variant, Qt::EditRole);
        return;
    }

    model->setData(index, value, Qt::EditRole);
}

void SqlQueryItemDelegate::setEditorDataForLineEdit(QLineEdit* le, const QModelIndex& index) const
{
    QVariant value = index.data(Qt::EditRole);
    if (value.userType() == QVariant::Double)
    {
        le->setText(doubleToString(value.toDouble()));
        return;
    }

    le->setText(value.toString());
}

SqlQueryItem* SqlQueryItemDelegate::getItem(const QModelIndex &index) const
{
    const SqlQueryModel* queryModel = dynamic_cast<const SqlQueryModel*>(index.model());
    return queryModel->itemFromIndex(index);
}

QWidget* SqlQueryItemDelegate::getEditor(int type, QWidget* parent) const
{
    UNUSED(type);
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(editor->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, 0, editor));
    return editor;

}

QString SqlQueryItemDelegate::getSqlForFkEditor(SqlQueryItem* item) const
{
    static_qstring(sql, "SELECT %1 FROM %2%3");
    static_qstring(srcColTpl, "%1 AS %2");
    static_qstring(dbColTpl, "%1.%2 AS %3");
    static_qstring(conditionTpl, "%1.%2 = %3.%4");
    static_qstring(conditionPrefixTpl, " WHERE %1");
    static_qstring(cellLimitTpl, "substr(%2, 0, %1)");

    QStringList selectedCols;
    QStringList fkConfitionTables;
    QStringList fkConditionCols;
    QStringList srcCols;
    Db* db = item->getModel()->getDb();
    Dialect dialect = db->getDialect();
    SchemaResolver resolver(db);

    QList<SqlQueryModelColumn::ConstraintFk*> fkList = item->getColumn()->getFkConstraints();
    int i = 0;
    QString src;
    QString fullSrcCol;
    QString col;
    for (SqlQueryModelColumn::ConstraintFk* fk : fkList)
    {
        col = wrapObjIfNeeded(fk->foreignColumn, dialect);
        src = wrapObjIfNeeded(fk->foreignTable, dialect);
        if (i == 0)
        {
            selectedCols << dbColTpl.arg(src, col,
                wrapObjIfNeeded(item->getColumn()->column, dialect));
        }

        srcCols = resolver.getTableColumns(src);
        for (const QString& srcCol : srcCols)
        {
            if (fk->foreignColumn.compare(srcCol, Qt::CaseInsensitive) == 0)
                continue; // Exclude matching column. We don't want the same column several times.

            fullSrcCol = src + "." + srcCol;
            selectedCols << srcColTpl.arg(cellLimitTpl.arg(CELL_LENGTH_LIMIT).arg(fullSrcCol), wrapObjName(fullSrcCol, dialect));
        }

        fkConditionCols << col;
        fkConfitionTables << src;

        i++;
    }

    QStringList conditions;
    QString firstSrc = wrapObjIfNeeded(fkConfitionTables.first(), dialect);
    QString firstCol = wrapObjIfNeeded(fkConditionCols.first(), dialect);
    for (i = 1; i < fkConfitionTables.size(); i++)
    {
        src = wrapObjIfNeeded(fkConfitionTables[i], dialect);
        col = wrapObjIfNeeded(fkConditionCols[i], dialect);
        conditions << conditionTpl.arg(firstSrc, firstCol, src, col);
    }

    QString conditionsStr;
    if (!conditions.isEmpty()) {
        conditionsStr = conditionPrefixTpl.arg(conditions.join(", "));
    }

    return sql.arg(selectedCols.join(", "), fkConfitionTables.join(", "), conditionsStr);
}

qlonglong SqlQueryItemDelegate::getRowCountForFkEditor(Db* db, const QString& query, bool* isError) const
{
    static_qstring(tpl, "SELECT count(*) FROM (%1)");

    QString sql = tpl.arg(query);
    SqlQueryPtr result = db->exec(sql);
    if (isError)
        *isError = result->isError();

    return result->getSingleCell().toLongLong();
}

void SqlQueryItemDelegate::fkDataReady()
{
    SqlQueryModel* model = dynamic_cast<SqlQueryModel*>(sender());
    SqlQueryView* queryView = model->getView();

    queryView->resizeColumnsToContents();
    queryView->resizeRowsToContents();

    int wd = queryView->horizontalHeader()->length();
    if (model->rowCount() > 10) // 10 is default visible item count for combobox
        wd += queryView->verticalScrollBar()->sizeHint().width();

    queryView->setMinimumWidth(wd);

    // Set selected combo value to initial value from the cell
    QComboBox* cb = modelToFkCombo[model];
    QVariant value = modelToFkInitialValue[model];
    if (model->rowCount() > 0)
    {
        QModelIndex startIdx = model->index(0, 0);
        QModelIndex endIdx = model->index(model->rowCount() - 1, 0);
        QModelIndexList idxList = model->findIndexes(startIdx, endIdx, SqlQueryItem::DataRole::VALUE, value, 1);

        if (idxList.size() > 0)
            cb->setCurrentIndex(idxList.first().row());
        else
            cb->setCurrentText(value.toString());
    }
    else
    {
        cb->setCurrentText(value.toString());
    }
}

void SqlQueryItemDelegate::fkDataFailed(const QString &errorText)
{
    notifyWarn(tr("Cannot edit this cell. Details: %1").arg(errorText));
}

QWidget* SqlQueryItemDelegate::getFkEditor(SqlQueryItem* item, QWidget* parent, const SqlQueryModel* model) const
{
    QString sql = getSqlForFkEditor(item);

    Db* db = model->getDb();
    bool countingError = false;
    qlonglong rowCount = getRowCountForFkEditor(db, sql, &countingError);
    if (rowCount > MAX_ROWS_FOR_FK)
    {
        notifyWarn(tr("Foreign key for column %2 has more than %1 possible values. It's too much to display in drop down list. You need to edit value manually.")
                   .arg(MAX_ROWS_FOR_FK).arg(item->getColumn()->column));

        return getEditor(item->getValue().userType(), parent);
    }

    if (rowCount == 0 && countingError && model->isStructureOutOfDate())
    {
        notifyWarn(tr("Cannot edit this cell. Details: %1").arg(tr("Structure of this table has changed since last data was loaded. Reload the data to proceed.")));
        return nullptr;
    }

    QComboBox *cb = new QComboBox(parent);
    cb->setEditable(true);

    // Prepare combo dropdown view.
    SqlQueryView* queryView = new SqlQueryView();
    queryView->setSimpleBrowserMode(true);
    connect(queryView->horizontalHeader(), &QHeaderView::sectionResized, [queryView](int, int, int)
    {
        int wd = queryView->horizontalHeader()->length();
        if (queryView->verticalScrollBar()->isVisible())
            wd += queryView->verticalScrollBar()->width();

        queryView->setMinimumWidth(wd);
    });

    SqlQueryModel* queryModel = new SqlQueryModel(queryView);
    queryModel->setView(queryView);

    // Mapping of model to cb, so we can update combo when data arrives.
    modelToFkInitialValue[queryModel] = item->getValue();
    modelToFkCombo[queryModel] = cb;
    connect(cb, &QComboBox::destroyed, [this, queryModel](QObject*)
    {
        modelToFkCombo.remove(queryModel);
        modelToFkInitialValue.remove(queryModel);
    });

    // When execution is done, update combo.
    connect(queryModel, SIGNAL(executionSuccessful()), this, SLOT(fkDataReady()));
    connect(queryModel, SIGNAL(executionFailed(QString)), this, SLOT(fkDataFailed(QString)));

    // Setup combo, model, etc.
    cb->setModel(queryModel);
    cb->setView(queryView);
    cb->setModelColumn(0);

    queryModel->setHardRowLimit(MAX_ROWS_FOR_FK);
    queryModel->setDb(db);
    queryModel->setQuery(sql);
    queryModel->executeQuery();

    queryView->verticalHeader()->setVisible(false);
    queryView->horizontalHeader()->setVisible(true);
    queryView->setSelectionMode(QAbstractItemView::SingleSelection);
    queryView->setSelectionBehavior(QAbstractItemView::SelectRows);

    return cb;
}

