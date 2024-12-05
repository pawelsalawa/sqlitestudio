#ifndef COLLATIONSEDITORMODEL_H
#define COLLATIONSEDITORMODEL_H

#include "services/collationmanager.h"
#include "guiSQLiteStudio_global.h"
#include <QIcon>
#include <QHash>
#include <QAbstractListModel>

class GUI_API_EXPORT CollationsEditorModel : public QAbstractListModel
{
        Q_OBJECT
    public:
        using QAbstractItemModel::setData;

        explicit CollationsEditorModel(QObject *parent = nullptr);

        void clearModified();
        bool isModified() const;
        bool isModified(int row) const;
        void setModified(int row, bool modified);
        void setName(int row, const QString& name);
        QString getName(int row) const;
        void setType(int row, CollationManager::CollationType type);
        CollationManager::CollationType getType(int row) const;
        void setLang(int row, const QString& lang);
        QString getLang(int row) const;
        void setAllDatabases(int row, bool allDatabases);
        bool getAllDatabases(int row) const;
        void setCode(int row, const QString& code);
        QString getCode(int row) const;
        void setDatabases(int row, const QStringList& databases);
        QStringList getDatabases(int row);
        bool isValid(int row) const;
        void setValid(int row, bool valid);
        bool isValid() const;
        void setData(const QList<CollationManager::CollationPtr>& collations);
        void addCollation(const CollationManager::CollationPtr& collation);
        void deleteCollation(int row);
        QList<CollationManager::CollationPtr> getCollations() const;
        QStringList getCollationNames() const;
        void validateNames();
        bool isAllowedName(int rowToSkip, const QString& nameToValidate);
        bool isValidRowIndex(int row) const;

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role) const;

    private:
        struct Collation
        {
            Collation();
            Collation(const CollationManager::CollationPtr& other);

            CollationManager::CollationPtr data;
            bool modified = false;
            bool valid = true;
            QString originalName;
        };

        void init();
        void emitDataChanged(int row);

        QList<Collation*> collationList;

        /**
         * @brief List of collation pointers before modifications.
         *
         * This list is kept to check for modifications in the overall list of collations.
         * Pointers on this list may be already deleted, so don't use them!
         * It's only used to compare list of pointers to collationList, so it can tell you
         * if the list was modified in regards of adding or deleting collations.
         */
        QList<Collation*> originalCollationList;
        QHash<QString,QIcon> langToIcon;
        bool listModified = false;
};

#endif // COLLATIONSEDITORMODEL_H
