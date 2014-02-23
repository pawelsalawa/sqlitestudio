#ifndef DBTREEITEMDELEGATE_H
#define DBTREEITEMDELEGATE_H

#include <QStyledItemDelegate>

class DbTreeItem;

class DbTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        explicit DbTreeItemDelegate(QObject *parent = 0);

        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        void paintDb(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintChildCount(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintTableLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintSystemIndexLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item) const;
        void paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, DbTreeItem* item, const QString& label) const;
};

#endif // DBTREEITEMDELEGATE_H
