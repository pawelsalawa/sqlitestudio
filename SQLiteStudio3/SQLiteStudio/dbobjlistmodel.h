#ifndef DBOBJLISTMODEL_H
#define DBOBJLISTMODEL_H

#include <QAbstractListModel>

class Db;

class DbObjListModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        enum class SortMode
        {
            LikeInDb,
            Alphabetical
        };

        enum class ObjectType
        {
            TABLE,
            INDEX,
            TRIGGER,
            VIEW,
            null
        };

        explicit DbObjListModel(QObject *parent = 0);

        QVariant data(const QModelIndex & index, int role) const;
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        QModelIndex sibling(int row, int column, const QModelIndex & idx) const;

        Db* getDb() const;
        void setDb(Db* value);

        SortMode getSortMode() const;
        void setSortMode(const SortMode& value);

        ObjectType getType() const;
        void setType(const ObjectType& value);

        bool getIncludeSystemObjects() const;
        void setIncludeSystemObjects(bool value);

    private:
        void updateList();
        QString typeString() const;

        ObjectType type = ObjectType::null;
        Db* db = nullptr;
        SortMode sortMode = SortMode::LikeInDb;
        QStringList objectList;
        QStringList unsortedObjectList;
        bool includeSystemObjects = true;
};

#endif // DBOBJLISTMODEL_H
