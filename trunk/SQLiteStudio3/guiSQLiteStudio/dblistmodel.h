#ifndef DBLISTMODEL_H
#define DBLISTMODEL_H

#include "db/db.h"
#include "guiSQLiteStudio_global.h"
#include <QAbstractListModel>

class QComboBox;

class GUI_API_EXPORT DbListModel : public QAbstractListModel
{
        Q_OBJECT
    public:
        enum class SortMode
        {
            LikeDbTree,
            Alphabetical,
            ConnectionOrder
        };

        explicit DbListModel(QObject *parent = 0);
        ~DbListModel();

        QVariant data(const QModelIndex & index, int role) const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QModelIndex sibling(int row, int column, const QModelIndex & idx) const;

        Db* getDb(int index);
        void setSortMode(SortMode sortMode);
        SortMode getSortMode() const;
        void setSortMode(const QString& sortMode);
        QString getSortModeString() const;
        void setCombo(QComboBox* combo);

    private:
        using QAbstractItemModel::sort;

        class DbTreeComparer
        {
            public:
                DbTreeComparer();
                bool operator()(Db* db1, Db* db2);

            private:
                QStringList dbTreeOrder;
        };

        class AlphaComparer
        {
            public:
                bool operator()(Db* db1, Db* db2);
        };

        void sort();

        QList<Db*> unsortedList;
        QList<Db*> dbList;
        SortMode sortMode = SortMode::ConnectionOrder;
        QComboBox* comboBox = nullptr;

    private slots:
        void dbConnected(Db* db);
        void dbDisconnected(Db* db);
};

#endif // DBLISTMODEL_H
