#ifndef DBTREEITEMDELEGATE_H
#define DBTREEITEMDELEGATE_H

#include "guiSQLiteStudio_global.h"
#include <QStyledItemDelegate>

class DbTreeView;
class DbTreeItem;

class GUI_API_EXPORT DbTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        explicit DbTreeItemDelegate(QWidget* parent = 0);

        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    private:
        void paintDb(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintChildCount(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintTableLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintVirtualTableLabel(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, DbTreeItem* item) const;
        void paintSystemIndexLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item, const QString& label) const;

    signals:
        void userEditCommitted(const QModelIndex &index, const QVariant &oldValue, const QVariant &newValue);
};

#endif // DBTREEITEMDELEGATE_H
