#ifndef SQLITEEXTENSIONEDITORMODEL_H
#define SQLITEEXTENSIONEDITORMODEL_H

#include "guiSQLiteStudio_global.h"
#include "services/sqliteextensionmanager.h"
#include <QIcon>
#include <QHash>
#include <QAbstractListModel>

class GUI_API_EXPORT SqliteExtensionEditorModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        using QAbstractItemModel::setData;

        explicit SqliteExtensionEditorModel(QObject* parent = nullptr);

        void clearModified();
        bool isModified() const;
        bool isModified(int row) const;
        void setModified(int row, bool modified);
        QString getName(int row) const;
        void setName(int row, const QString& name);
        void setFilePath(int row, const QString& filePath);
        QString getFilePath(int row) const;
        void setInitFunction(int row, const QString& initFunc);
        QString getInitFunction(int row) const;
        void setAllDatabases(int row, bool allDatabases);
        bool getAllDatabases(int row) const;
        void setDatabases(int row, const QStringList& databases);
        QStringList getDatabases(int row);
        bool isValid(int row) const;
        void setValid(int row, bool valid);
        bool isValid() const;
        void setData(const QList<SqliteExtensionManager::ExtensionPtr>& extensions);
        void addExtension(const SqliteExtensionManager::ExtensionPtr& extension);
        void deleteExtension(int row);
        QList<SqliteExtensionManager::ExtensionPtr> getExtensions() const;
        bool isValidRowIndex(int row) const;

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role) const;

    private:
        struct Extension
        {
            Extension();
            Extension(const SqliteExtensionManager::ExtensionPtr& other);

            SqliteExtensionManager::ExtensionPtr data;
            QString name;
            bool modified = false;
            bool valid = true;
        };

        void emitDataChanged(int row);

        QList<Extension*> extensionList;
        QList<Extension*> originalExtensionList;
        QHash<QString, QIcon> langToIcon;
        bool listModified = false;
};

#endif // SQLITEEXTENSIONEDITORMODEL_H
