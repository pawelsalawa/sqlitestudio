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
#include <limits>
#include <QToolTip>
#include <QTextLayout>
#include <QtMath>
#include <QScreen>

bool SqlQueryItemDelegate::warnedAboutHugeContents = false;

SqlQueryItemDelegate::SqlQueryItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    fullValueButtonOption.icon = ICONS.LOAD_FULL_VALUE;
    fullValueButtonOption.iconSize = QSize(LOAD_FULL_VALUE_ICON_SIZE, LOAD_FULL_VALUE_ICON_SIZE);
    fullValueButtonOption.state = QStyle::State_Enabled;
}

void SqlQueryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    SqlQueryItem* item = getItem(index);

    if (item->isUncommitted())
    {
        painter->setPen(item->isCommittingError() ? QColor(Qt::red) : QColor(Qt::blue));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width()-1, option.rect.height()-1);
    }

    if (item->isLimitedValue())
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        QString text = displayText(item->getValue(), opt.locale);
        int textWidth = opt.fontMetrics.horizontalAdvance(text);
        int margin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, opt.widget) + 1; // from QCommonStyle source code
        if (opt.rect.width() >= (textWidth + LOAD_FULL_VALUE_BUTTON_SIZE + margin))
        {
            QStyleOptionButton button = fullValueButtonOption;
            button.rect = getLoadFullValueButtonRegion(opt.rect);
            button.state = QStyle::State_Enabled | QStyle::State_MouseOver;
            if (lmbPressedOnButton)
                button.state |= QStyle::State_Sunken | QStyle::State_Active;

            QApplication::style()->drawControl(
                        (mouseOverFullDataButton == index) ? QStyle::CE_PushButton : QStyle::CE_PushButtonLabel,
                        &button, painter);
        }
    }
}

bool SqlQueryItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    switch (event->type())
    {
        case QEvent::MouseButtonDblClick:
            if (isOverFullValueButton(option.rect, dynamic_cast<QMouseEvent*>(event)) && isLimited(index))
                return true;

            break;
        case QEvent::MouseButtonPress:
        {
            if (isOverFullValueButton(option.rect, dynamic_cast<QMouseEvent*>(event)) && isLimited(index))
            {
                lmbPressedOnButton = true;
                return true;
            }

            break;
        }
        case QEvent::MouseButtonRelease:
            if (lmbPressedOnButton && isOverFullValueButton(option.rect, dynamic_cast<QMouseEvent*>(event)) && isLimited(index))
            {
                lmbPressedOnButton = false;
                getItem(index)->loadFullData();
                return true;
            }
            lmbPressedOnButton = false;
            break;
        case QEvent::MouseMove:
        {
            bool isOverButton = isOverFullValueButton(option.rect, dynamic_cast<QMouseEvent*>(event));
            if (mouseOverFullDataButton.isValid() != isOverButton)
            {
                mouseOverFullDataButton = isOverButton ? index : QModelIndex();
                dynamic_cast<SqlQueryModel*>(model)->getView()->update(index);
            }

            if (!isOverButton && showingFullButtonTooltip)
            {
                QToolTip::hideText();
                showingFullButtonTooltip = false;
            }
            break;
        }
        default:
            break;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool SqlQueryItemDelegate::shouldLoadFullData(const QRect& rect, QMouseEvent* event, const QModelIndex& index)
{
    return shouldLoadFullData(rect, event->x(), event->y(), index);
}

bool SqlQueryItemDelegate::shouldLoadFullData(const QRect& rect, int x, int y, const QModelIndex& index)
{
    return isOverFullValueButton(rect, x, y) && isLimited(index);
}

void SqlQueryItemDelegate::mouseLeftIndex(const QModelIndex& index)
{
    if (mouseOverFullDataButton == index)
        mouseOverFullDataButton = QModelIndex();
}

bool SqlQueryItemDelegate::isLimited(const QModelIndex& index)
{
    return index.data(SqlQueryItem::DataRole::LIMITED_VALUE).toBool();
}

bool SqlQueryItemDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (shouldLoadFullData(option.rect, event->x(), event->y(), index))
    {
        QToolTip::showText(view->mapToGlobal(event->pos() - QPoint(0, 15)), tr("Load remaining part of the value"));
        showingFullButtonTooltip = true;
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
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

    if (item->isLimitedValue() && !item->loadFullData().isNull() && model->isStructureOutOfDate())
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
        return doubleToString(value);

    return QStyledItemDelegate::displayText(value, locale);
}

void SqlQueryItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    // No need to check or load full data, it is already preloaded if necessary in createEditor().
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
    UNUSED(cb);
    UNUSED(index);
    // There used to be code here, but it's empty now.
    // All necessary data population happens in the fkDataReady().
    // Keeping this method just for this comment and for consistency across different kind of cell editors
    // (i.e. each editor has method to copy value from model to editor and another to copy from editor to model).
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

    SqlQueryModel* dataModel = dynamic_cast<SqlQueryModel*>(model);
    SqlQueryItem* theItem = dataModel->itemFromIndex(index);
    if (!theItem)
    {
        qCritical() << "Confirmed FK edition, but there is no SqlQueryItem for which this was triggered!" << index;
        return;
    }

    int idx = cb->currentIndex();
    if (idx < 0 || idx >= cbModel->rowCount())
    {
        theItem->setValue(cbText);
        return;
    }

    QList<SqlQueryItem *> row = cbModel->getRow(idx);
    if (row.size() < 2 || !row[1])
    {
        // This happens when inexisting value is confirmed with "Enter" key,
        // cause rowCount() is apparently incremented, but items not yet.
        qCritical() << "Confirmed FK edition, but there is no item in the row for index" << idx << ", CB row count is" << cbModel->rowCount();
        theItem->setValue(cbText);
        return;
    }

    QVariant comboData = row[1]->getFullValue();
    theItem->setValue(comboData);
}

void SqlQueryItemDelegate::setModelDataForLineEdit(QLineEdit* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QString value = editor->text();

    if (CFG_UI.General.KeepNullWhenEmptyValue.get() && model->data(index, Qt::EditRole).isNull() && value.isEmpty())
        return;

    const SqlQueryModel* queryModel = dynamic_cast<const SqlQueryModel*>(model);
    SqlQueryItem* item = queryModel->itemFromIndex(index);

    if (item->getColumn()->dataType.isNumeric())
    {
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
    }
    model->setData(index, value, Qt::EditRole);
}

void SqlQueryItemDelegate::setEditorDataForLineEdit(QLineEdit* le, const QModelIndex& index) const
{
    QVariant value = index.data(Qt::EditRole);
    if (value.userType() == QVariant::Double)
    {
        le->setText(doubleToString(value));
        return;
    }

    QString str = value.toString();
    if (str.size() > HUGE_CONTENTS_WARNING_LIMIT && !warnedAboutHugeContents)
    {
        NOTIFY_MANAGER->warn(tr("Editing a huge contents in an inline cell editor is not a good idea. It can become slow and inconvenient. It's better to edit such big contents in a Form View, or in popup editor (available under rick-click menu)."));
        warnedAboutHugeContents = true;
    }

    le->setText(str);
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
    editor->setMaxLength(std::numeric_limits<int>::max());
    editor->setFrame(editor->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, 0, editor));
    return editor;
}

QString SqlQueryItemDelegate::getSqlForFkEditor(SqlQueryItem* item) const
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
    Db* db = item->getModel()->getDb();
    SchemaResolver resolver(db);

    QList<SqlQueryModelColumn::ConstraintFk*> fkList = item->getColumn()->getFkConstraints();
    int i = 0;
    QString src;
    QString fullSrcCol;
    QString col;
    QString firstSrcCol;
    QStringList usedNames;
    for (SqlQueryModelColumn::ConstraintFk* fk : fkList)
    {
        col = wrapObjIfNeeded(fk->foreignColumn);
        src = wrapObjIfNeeded(fk->foreignTable);
        if (i == 0)
        {
            firstSrcCol = col;
            fullSrcCol = src + "." + col;
            selectedCols << dbColTpl.arg(fullSrcCol, wrapObjIfNeeded(item->getColumn()->column));
        }

        if (fkConditionTables.contains(src, Qt::CaseInsensitive))
            continue;

        srcCols = resolver.getTableColumns(src);
        for (const QString& srcCol : srcCols)
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
    QVariant fullValue = item->getFullValue();
    QString currValueColName = generateUniqueName("curr", usedNames);
    QString currValueExpr = fullValue.isNull() ?
                currNullValueTpl.arg(firstSrcCol, currValueColName) :
                currValueTpl.arg(firstSrcCol, wrapValueIfNeeded(fullValue), currValueColName);

    return sql.arg(
                selectedCols.join(", "),
                fkConditionTables.join(", "),
                conditionsStr,
                currValueExpr
                );
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

QRect SqlQueryItemDelegate::getLoadFullValueButtonRegion(const QRect& cell)
{
    int x = cell.left() + cell.width() - LOAD_FULL_VALUE_BUTTON_SIZE - LOAD_FULL_VALUE_BUTTON_SIDE_MARGIN;
    int y = cell.top() + (cell.height() / 2) - (LOAD_FULL_VALUE_BUTTON_SIZE / 2);
    return QRect(x, y, LOAD_FULL_VALUE_BUTTON_SIZE, LOAD_FULL_VALUE_BUTTON_SIZE);
}

bool SqlQueryItemDelegate::isOverFullValueButton(const QRect& cell, QMouseEvent* event)
{
    return isOverFullValueButton(cell, event->x(), event->y());
}

bool SqlQueryItemDelegate::isOverFullValueButton(const QRect& cell, int x, int y)
{
    QRect buttonRect = getLoadFullValueButtonRegion(cell);
    return buttonRect.contains(x, y);
}

int SqlQueryItemDelegate::getFkViewHeaderWidth(SqlQueryView* fkView, bool includeScrollBar) const
{
    int wd = fkView->horizontalHeader()->length();
    if (includeScrollBar && fkView->verticalScrollBar()->isVisible())
        wd += fkView->verticalScrollBar()->width();

    return wd;
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
    SqlQueryView* comboView = new SqlQueryView();
    comboView->setSimpleBrowserMode(true);
    comboView->setMaximumWidth(QGuiApplication::primaryScreen()->size().width());

    fkViewParentItemSize = model->getView()->horizontalHeader()->sectionSize(item->index().column());
    connect(comboView->horizontalHeader(), &QHeaderView::sectionResized, [this, comboView, model](int, int, int)
    {
        if (!model->isAllDataLoaded())
            return;

        updateComboViewGeometry(comboView, false);
    });

    SqlQueryModel* comboModel = new SqlQueryModel(comboView);
    comboModel->setView(comboView);

    // Mapping of model to cb, so we can update combo when data arrives.
    modelToFkInitialValue[comboModel] = item->getFullValue();
    modelToFkCombo[comboModel] = cb;
    connect(cb, &QComboBox::destroyed, [this, comboModel](QObject*)
    {
        modelToFkCombo.remove(comboModel);
        modelToFkInitialValue.remove(comboModel);
    });

    connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, comboModel](int idx)
    {
        if (idx > -1 && comboModel->isAllDataLoaded())
            comboModel->itemFromIndex(idx, 1)->loadFullData();
    });

    // When execution is done, update combo.
    connect(comboModel, SIGNAL(executionSuccessful()), this, SLOT(fkDataReady()));
    connect(comboModel, SIGNAL(executionFailed(QString)), this, SLOT(fkDataFailed(QString)));

    // Setup combo, model, etc.
    cb->setModel(comboModel);
    cb->setView(comboView);
    cb->setModelColumn(1);
    cb->view()->viewport()->installEventFilter(new FkComboFilter(this, comboView, cb));
    cb->view()->viewport()->installEventFilter(new FkComboShowFilter(this, comboView, cb));
    cb->view()->verticalScrollBar()->installEventFilter(new FkComboShowFilter(this, comboView, cb));

    comboModel->setHardRowLimit(MAX_ROWS_FOR_FK);
    comboModel->setCellDataLengthLimit(FK_CELL_LENGTH_LIMIT);
    comboModel->setDb(db);
    comboModel->setQuery(sql);
    comboModel->setAsyncMode(false);
    comboModel->executeQuery();

    comboView->verticalHeader()->setVisible(false);
    comboView->horizontalHeader()->setVisible(true);
    comboView->setSelectionMode(QAbstractItemView::SingleSelection);
    comboView->setSelectionBehavior(QAbstractItemView::SelectRows);

    return cb;
}

void SqlQueryItemDelegate::fkDataReady()
{
    SqlQueryModel* model = dynamic_cast<SqlQueryModel*>(sender());
    SqlQueryView* queryView = model->getView();

    queryView->horizontalHeader()->setSectionHidden(0, true);
    queryView->resizeColumnsToContents();
    queryView->resizeRowsToContents();

    updateComboViewGeometry(queryView, true);

    // Set selected combo value to initial value from the cell
    QComboBox* cb = modelToFkCombo[model];
    QVariant valueFromQueryModel = modelToFkInitialValue[model];

    if (model->rowCount() > 0)
    {
        QModelIndex startIdx = model->index(0, 0);
        QModelIndex endIdx = model->index(model->rowCount() - 1, 0);
        QModelIndexList idxList = model->findIndexes(startIdx, endIdx, SqlQueryItem::DataRole::VALUE, 1, 1);

        if (idxList.size() > 0)
        {
            model->itemFromIndex(idxList.first().row(), 1)->loadFullData();
            cb->setCurrentIndex(idxList.first().row());
        }
        else
            cb->setCurrentText(valueFromQueryModel.toString());
    }
    else
        cb->setCurrentText(valueFromQueryModel.toString());
}

void SqlQueryItemDelegate::fkDataFailed(const QString &errorText)
{
    notifyWarn(tr("Cannot edit this cell. Details: %1").arg(errorText));
}

void SqlQueryItemDelegate::updateComboViewGeometry(SqlQueryView* comboView, bool initial) const
{
    int wd = getFkViewHeaderWidth(comboView, true);
    comboView->setMinimumWidth(qMin(qMax(fkViewParentItemSize, wd), comboView->maximumWidth()));

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


SqlQueryItemDelegate::FkComboFilter::FkComboFilter(const SqlQueryItemDelegate* delegate, SqlQueryView* comboView, QObject* parent)
    : QObject(parent), delegate(delegate), comboView(comboView)
{
}

bool SqlQueryItemDelegate::FkComboFilter::eventFilter(QObject* obj, QEvent* event)
{
    UNUSED(obj);
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* e = dynamic_cast<QMouseEvent*>(event);
        QModelIndex idx = comboView->indexAt(QPoint(e->pos()));
        if (!idx.isValid())
            return false;

        SqlQueryItem* item = comboView->getModel()->itemFromIndex(idx);
        if (shouldLoadFullData(comboView->visualRect(idx), e, idx))
        {
            item->loadFullData();
            return true;
        }
    }
    return false;
}

SqlQueryItemDelegate::FkComboShowFilter::FkComboShowFilter(const SqlQueryItemDelegate* delegate, SqlQueryView* comboView, QObject* parent)
    : QObject(parent), delegate(delegate), comboView(comboView)
{
}

bool SqlQueryItemDelegate::FkComboShowFilter::eventFilter(QObject* obj, QEvent* event)
{
    UNUSED(obj);
    if (event->type() == QEvent::Show)
        delegate->updateComboViewGeometry(comboView, true);

    return false;
}
