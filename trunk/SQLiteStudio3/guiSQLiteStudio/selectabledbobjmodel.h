#ifndef SELECTABLEDBOBJMODEL_H
#define SELECTABLEDBOBJMODEL_H

#include "guiSQLiteStudio_global.h"
#include <QSortFilterProxyModel>
#include <QSet>

class DbTreeItem;
class QTreeView;

class GUI_API_EXPORT SelectableDbObjModel : public QSortFilterProxyModel
{
        Q_OBJECT
    public:
        explicit SelectableDbObjModel(QObject *parent = 0);

        QVariant data(const QModelIndex& index, int role) const;
        bool setData(const QModelIndex& index, const QVariant& value, int role);
        Qt::ItemFlags flags(const QModelIndex& index) const;

        QString getDbName() const;
        void setDbName(const QString& value);

        QStringList getCheckedObjects() const;
        void setCheckedObjects(const QStringList& value);

        void setRootChecked(bool checked);
        DbTreeItem* getItemForIndex(const QModelIndex& index) const;

    protected:
        bool filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const;

    private:
        DbTreeItem* getItemForProxyIndex(const QModelIndex& index) const;
        Qt::CheckState getStateFromChilds(const QModelIndex& index) const;
        void setRecurrently(const QModelIndex& index, Qt::CheckState checked);
        bool isObject(DbTreeItem* item) const;
        bool checkRecurrentlyForDb(DbTreeItem* item) const;

        QSet<QString> checkedObjects;
        QString dbName;
};

#endif // SELECTABLEDBOBJMODEL_H
