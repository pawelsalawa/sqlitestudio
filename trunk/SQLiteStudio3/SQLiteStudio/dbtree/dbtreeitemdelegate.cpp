#include "dbtreeitemdelegate.h"
#include "dbtreeitem.h"
#include "dbtreemodel.h"
#include "common/utils_sql.h"
#include "uiconfig.h"
#include <QPainter>
#include <QDebug>

DbTreeItemDelegate::DbTreeItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize DbTreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(18);
    return size;
}

void DbTreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    opt.font = CFG_UI.Fonts.DbTree.get();
    opt.fontMetrics = QFontMetrics(opt.font);

    QStyledItemDelegate::paint(painter, opt, index);

    if (!CFG_UI.General.ShowDbTreeLabels.get())
        return;

    const DbTreeModel* model = dynamic_cast<const DbTreeModel*>(index.model());
    DbTreeItem* item = dynamic_cast<DbTreeItem*>(model->itemFromIndex(index));

    switch (item->getType())
    {
        case DbTreeItem::Type::DIR:
            break;
        case DbTreeItem::Type::DB:
        case DbTreeItem::Type::INVALID_DB:
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
        case DbTreeItem::Type::INDEX:
            paintSystemIndexLabel(painter, opt, index, item);
            break;
        case DbTreeItem::Type::TRIGGER:
        case DbTreeItem::Type::VIEW:
        case DbTreeItem::Type::COLUMN:
        case DbTreeItem::Type::ITEM_PROTOTYPE:
            break;
    }
}

void DbTreeItemDelegate::paintDb(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem *item) const
{
    static const QString versionStringTemplate = QStringLiteral("(%1)");
    QString versionString = versionStringTemplate.arg("?");
    if (item->getType() == DbTreeItem::Type::INVALID_DB)
    {
        versionString = versionStringTemplate.arg(tr("error", "dbtree labels"));
    }
    else
    {
        Db* db = item->getDb();
        if (!db)
            return;

        QString t = db->getTypeLabel();
        versionString = versionStringTemplate.arg(t);
    }

    paintLabel(painter, option, index, item, versionString);
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

    if (!CFG_UI.General.ShowRegularTableLabels.get())
        return;

    int columnsCount = item->child(0)->rowCount();
    int indexesCount = item->child(1)->rowCount();
    int triggersCount = item->child(2)->rowCount();
    paintLabel(painter, option, index, item, QString("(%1, %2, %3)").arg(columnsCount).arg(indexesCount).arg(triggersCount));
}

void DbTreeItemDelegate::paintSystemIndexLabel(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, DbTreeItem* item) const
{
    Db* db = item->getDb();
    if (!db)
        return;

    if (!isSystemIndex(item->text(), db->getDialect()))
        return;

    paintLabel(painter, option, index, item, tr("(system index)", "database tree label"));
}

void DbTreeItemDelegate::paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem *item, const QString &label) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    // Colors
    painter->setPen(CFG_UI.Colors.DbTreeLabelsFg.get());

    // Font
    opt.font = CFG_UI.Fonts.DbTreeLabel.get();
    opt.fontMetrics = QFontMetrics(opt.font);
    painter->setFont(opt.font);

    // Coords
    int x = option.rect.x() + option.fontMetrics.width(item->text()) + 15 + option.decorationSize.width();
    int y = opt.rect.y() + (opt.rect.height() - opt.fontMetrics.descent() - opt.fontMetrics.ascent()) / 2 + opt.fontMetrics.ascent();

    // Paint
    painter->drawText(QPoint(x, y), label);
    painter->restore();
}
