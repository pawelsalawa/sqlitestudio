#ifndef SELECTABLEDBMODEL_H
#define SELECTABLEDBMODEL_H

#include "guiSQLiteStudio_global.h"
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QVariant>

class DbTreeItem;

class GUI_API_EXPORT SelectableDbModel : public QSortFilterProxyModel
{
    public:
        explicit SelectableDbModel(QObject *parent = 0);

        QVariant data(const QModelIndex& index, int role) const;
        bool setData(const QModelIndex& index, const QVariant& value, int role);
        Qt::ItemFlags flags(const QModelIndex& index) const;

        void setDatabases(const QStringList& databases);
        QStringList getDatabases() const;

        int getDisabledVersion() const;
        void setDisabledVersion(int value);

    protected:
        bool filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const;

    private:
        DbTreeItem* getItemForProxyIndex(const QModelIndex& index) const;

        QStringList checkedDatabases;
        int disabledVersion = -1;
};

#endif // SELECTABLEDBMODEL_H
