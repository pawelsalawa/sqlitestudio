#ifndef SQLQUERYITEMDELEGATE_H
#define SQLQUERYITEMDELEGATE_H

#include "guiSQLiteStudio_global.h"
#include "db/sqlquery.h"
#include <QStyledItemDelegate>

class SqlQueryItem;
class QComboBox;
class QStandardItemModel;

class GUI_API_EXPORT SqlQueryItemDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit SqlQueryItemDelegate(QObject *parent = 0);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QString	displayText(const QVariant & value, const QLocale & locale) const;
        void setEditorData(QWidget * editor, const QModelIndex & index) const;
        void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;

    private:
        SqlQueryItem* getItem(const QModelIndex &index) const;
        QWidget* getEditor(int type, QWidget* parent) const;
        QWidget* getFkEditor(SqlQueryItem* item, QWidget* parent) const;
        void setEditorDataForFk(QComboBox* cb, const QModelIndex& index) const;
        void setModelDataForFk(QComboBox* editor, QAbstractItemModel* model, const QModelIndex& index) const;
        QString getSqlForFkEditor(SqlQueryItem* item) const;
        void copyToModel(const SqlQueryPtr& results, QStandardItemModel* model) const;
};

#endif // SQLQUERYITEMDELEGATE_H
