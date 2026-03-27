#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include "services/exportmanager.h"
#include "db/queryexecutor.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QObject>
#include <QRunnable>
#include <QMutex>

class Db;

class API_EXPORT ExportWorker : public QObject, public QRunnable
{
        Q_OBJECT
    public:
        ExportWorker(ExportPlugin* plugin, ExportManager::StandardExportConfig* config, QIODevice* output, QObject *parent = 0);
        ~ExportWorker();

        void run();
        void prepareExportQueryResults(Db* db, const QString& query);
        void prepareExportDatabase(Db* db, const QStringList& objectListToExport);
        void prepareExportTable(Db* db, const QString& database, const QString& table);
        void prepareExportView(Db* db, const QString& database, const QString& view);

    private:
        void prepareParser();
        bool exportQueryResults();
        QHash<ExportManager::ExportProviderFlag, QVariant> getProviderDataForQueryResults();
        bool exportDatabase();
        bool exportDatabaseObjects(const QList<ExportManager::ExportObjectPtr>& dbObjects, ExportManager::ExportObject::Type type);
        bool exportTable();
        bool exportView();
        bool exportTableInternal(const QString& database, const QString& table, const QString& ddl, SqliteQueryPtr parsedDdl, SqlQueryPtr results,
                                 const QHash<ExportManager::ExportProviderFlag, QVariant>& providerData);
        bool exportViewInternal(const QString& database, const QString& view, const QString& ddl, SqliteQueryPtr parsedDdl, SqlQueryPtr results,
                                 const QHash<ExportManager::ExportProviderFlag, QVariant>& providerData);
        QList<ExportManager::ExportObjectPtr> collectDbObjects(QString* errorMessage);
        void queryTableOrViewDataToExport(Db* db, const QString& tableOrView, SqlQueryPtr& dataPtr, QHash<ExportManager::ExportProviderFlag, QVariant>& providerData,
                                          QString* errorMessage, bool isTable) const;
        bool isInterrupted();
        void logExportFail(const QString& stageName);

        ExportPlugin* plugin = nullptr;
        ExportManager::StandardExportConfig* config = nullptr;
        QIODevice* output = nullptr;
        ExportManager::ExportMode exportMode = ExportManager::UNDEFINED;
        Db* db = nullptr;
        QString query;
        QString database;
        QString table;
        QString view;
        QStringList objectListToExport;
        QueryExecutor* executor = nullptr;
        bool interrupted = false;
        QMutex interruptMutex;
        Parser* parser = nullptr;

    public slots:
        void interrupt();

    signals:
        void finished(bool result, QIODevice* output);
        void finishedStep(int step);
};

#endif // EXPORTWORKER_H
