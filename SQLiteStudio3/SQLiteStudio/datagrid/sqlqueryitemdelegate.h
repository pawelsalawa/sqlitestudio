#ifndef SQLQUERYITEMDELEGATE_H
#define SQLQUERYITEMDELEGATE_H

#include <QStyledItemDelegate>

class SqlQueryItem;

class SqlQueryItemDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit SqlQueryItemDelegate(QObject *parent = 0);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:
        SqlQueryItem* getItem(const QModelIndex &index) const;
        QWidget* getEditor(int type, QWidget* parent) const;
};

#endif // SQLQUERYITEMDELEGATE_H
