#ifndef IMPORTWORKER_H
#define IMPORTWORKER_H

#include "services/importmanager.h"
#include <QObject>
#include <QRunnable>
#include <QMutex>

class ImportWorker : public QObject, public QRunnable
{
        Q_OBJECT
    public:
        ImportWorker(ImportPlugin* plugin, ImportManager::StandardImportConfig* config, Db* db, const QString& table, QObject *parent = 0);

        void run();

    private:
        void readPluginColumns();
        void error(const QString& err);
        bool prepareTable();
        bool importData(int& rowCount);
        bool isInterrupted();

        ImportPlugin* plugin = nullptr;
        ImportManager::StandardImportConfig* config = nullptr;
        Db* db = nullptr;
        QString table;
        QStringList columnsFromPlugin;
        QStringList columnTypesFromPlugin;
        QStringList tableColumns;
        QStringList targetColumns;
        bool interrupted = false;
        QMutex interruptMutex;
        bool tableCreated = false;
        bool shouldSkipTransaction = false;

    public slots:
        void interrupt();

    signals:
        void createdTable(Db* db, const QString& table);
        void finished(bool result, int rowCount);
};

#endif // IMPORTWORKER_H
