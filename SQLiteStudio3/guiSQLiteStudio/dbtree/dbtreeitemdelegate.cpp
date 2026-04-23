#include "dbtreeitemdelegate.h"
#include "dbtreeitem.h"
#include "dbtreemodel.h"
#include "common/utils_sql.h"
#include "uiconfig.h"
#include "dbtree.h"
#include "dbtreeview.h"
#include <QPainter>
#include <QDebug>

DbTreeItemDelegate::DbTreeItemDelegate(QWidget* parent) :
    QStyledItemDelegate(parent)
{
}

QSize DbTreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    QFont f = CFG_UI.Fonts.DbTree.get();
    QFontMetrics fm(f);
    size.setHeight(qMax(16, fm.height() + 4));
    return size;
}

void DbTreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const DbTreeModel* model = dynamic_cast<const DbTreeModel*>(index.model());
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(model->itemFromIndex(index));

    opt.font = CFG_UI.Fonts.DbTree.get();
    opt.fontMetrics = QFontMetrics(opt.font);

    QModelIndex currIndex = DBTREE->getView()->selectionModel()->currentIndex();
    if (currIndex.isValid() && item->index() == currIndex)
        opt.state |= QStyle::State_HasFocus;

    QStyledItemDelegate::paint(painter, opt, index);

    if (!CFG_UI.DbList.ShowDbTreeLabels.get())
        return;

    switch (item->getType())
    {
        case DbTreeItem::Type::DIR:
            break;
        case DbTreeItem::Type::DB:
            paintDb(painter, opt, index, item);
            break;
        case DbTreeItem::Type::TABLES:
        case DbTreeItem::Type::INDEXES:
        case DbTreeItem::Type::TRIGGERS:
        case DbTreeItem::Type::VIEWS:
        case DbTreeItem::Type::COLUMNS:
            paintChildCount(painter, opt, index, item);
            break;
        case DbTreeItem::Type::TABLE:
            paintTableLabel(painter, opt, index, item);
            break;
        case DbTreeItem::Type::VIRTUAL_TABLE:
            paintVirtualTableLabel(painter, opt, index, item);
            break;
        case DbTreeItem::Type::INDEX:
            paintSystemIndexLabel(painter, opt, index, item);
            break;
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEW:
        case DbTreeItem::Type::COLUMN:
            paintColumnTypeLabel(painter, opt, index, item);
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            break;
    }
}

void DbTreeItemDelegate::paintDb(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem *item) const
{
    static const QString formatStringTemplate = QStringLiteral("(%1)");
    QString formatString = formatStringTemplate.arg("?");
    Db* db = item->getDb();
    if (!db)
        return;

    if (db->isValid())
    {
        QString reportedFormat = db->getTypeLabel();
        int colonIdx = reportedFormat.indexOf(":");
        if (colonIdx > -1)
            reportedFormat = reportedFormat.mid(0, colonIdx);

        formatString = formatStringTemplate.arg(reportedFormat);
    }
    else
    {
        formatString = formatStringTemplate.arg(tr("error", "dbtree labels"));
    }

    paintLabel(painter, option, index, item, formatString);
}

void DbTreeItemDelegate::paintChildCount(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem *item) const
{
    int cnt = item->rowCount();
    if (cnt > 0)
        paintLabel(painter, option, index, item, QString("(%1)").arg(cnt));
}

void DbTreeItemDelegate::paintTableLabel(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, DbTreeItem* item) const
{
    if (isSystemTable(item->text()))
    {
        paintLabel(painter, option, index, item, tr("(system table)", "database tree label"));
        return;
    }

    if (!CFG_UI.DbList.ShowRegularTableLabels.get())
        return;

    int columnsCount = item->child(0)->rowCount();
    int indexesCount = item->child(1)->rowCount();
    int triggersCount = item->child(2)->rowCount();
    paintLabel(painter, option, index, item, QString("(%1, %2, %3)").arg(columnsCount).arg(indexesCount).arg(triggersCount));
}

void DbTreeItemDelegate::paintVirtualTableLabel(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, DbTreeItem* item) const
{
    if (!CFG_UI.DbList.ShowVirtualTableLabels.get())
        return;

    paintLabel(painter, option, index, item, tr("(virtual)", "virtual table label"));
}

void DbTreeItemDelegate::paintSystemIndexLabel(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, DbTreeItem* item) const
{
    Db* db = item->getDb();
    if (!db || !db->isValid())
        return;

    if (!isSystemIndex(item->text()))
        return;

    paintLabel(painter, option, index, item, tr("(system index)", "database tree label"));
}

void DbTreeItemDelegate::paintColumnTypeLabel(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, DbTreeItem* item) const
{
    if (!CFG_UI.DbList.ShowColumnTypeLabels.get())
        return;

    Db* db = item->getDb();
    if (!db || !db->isValid())
        return;

    QString type = item->getColumnType();
    if (type.isEmpty())
        return;

    paintLabel(painter, option, index, item, QString(": %1").arg(type));
}

void DbTreeItemDelegate::paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem *item, const QString &label) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    // Colors
    painter->setPen(QApplication::style()->standardPalette().link().color());

    // Font
    opt.font = CFG_UI.Fonts.DbTreeLabel.get();
    opt.fontMetrics = QFontMetrics(opt.font);
    painter->setFont(opt.font);

    // Coords
    int x = option.rect.x() + option.fontMetrics.horizontalAdvance(item->text()) + 15 + option.decorationSize.width();
    int y = opt.rect.y() + (opt.rect.height() - opt.fontMetrics.descent() - opt.fontMetrics.ascent()) / 2 + opt.fontMetrics.ascent();

    // Paint
    painter->drawText(QPoint(x, y), label);
    painter->restore();
}

void DbTreeItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QVariant oldValue = model->data(index, Qt::EditRole);
    QStyledItemDelegate::setModelData(editor, model, index);
    QVariant newValue = model->data(index, Qt::EditRole);
    emit const_cast<DbTreeItemDelegate*>(this)->userEditCommitted(index, oldValue, newValue);
}

QWidget* DbTreeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
    if (editor)
        editor->setFont(CFG_UI.Fonts.DbTreeLabel.get());

    return editor;
}
