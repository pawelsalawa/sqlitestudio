#ifndef SQLITEEXTENSIONMANAGER_H
#define SQLITEEXTENSIONMANAGER_H

#include "coreSQLiteStudio_global.h"
#include "sqlitestudio.h"
#include <QSharedPointer>
#include <QObject>

class API_EXPORT SqliteExtensionManager : public QObject
{
    Q_OBJECT

    public:
        struct API_EXPORT Extension
        {
            QString filePath;
            QString initFunc;
            QStringList databases;
            bool allDatabases = true;
        };

        typedef QSharedPointer<Extension> ExtensionPtr;

        virtual void setExtensions(const QList<ExtensionPtr>& newExtensions) = 0;
        virtual QList<ExtensionPtr> getAllExtensions() const = 0;
        virtual QList<ExtensionPtr> getExtensionForDatabase(const QString& dbName) const = 0;
        virtual QStringList getExtensionDirs() const = 0;

    signals:
        void extensionListChanged();
};

#define SQLITE_EXTENSIONS SQLITESTUDIO->getSqliteExtensionManager()

#endif // SQLITEEXTENSIONMANAGER_H
