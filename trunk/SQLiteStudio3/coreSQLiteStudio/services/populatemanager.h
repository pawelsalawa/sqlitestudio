#ifndef POPULATEMANAGER_H
#define POPULATEMANAGER_H

#include "pluginservicebase.h"
#include "sqlitestudio.h"
#include <QObject>
#include <QHash>
#include <QStringList>

class PopulatePlugin;
class PopulateEngine;
class Db;

class API_EXPORT PopulateManager : public PluginServiceBase
{
        Q_OBJECT

    public:
        explicit PopulateManager(QObject *parent = 0);

        void populate(Db* db, const QString& table, const QHash<QString, PopulateEngine*>& engines, qint64 rows);

    private:
        void error();
        void deleteEngines(const QList<PopulateEngine*>& engines);

        bool workInProgress = false;
        Db* db = nullptr;
        QString table;
        QStringList columns;
        QList<PopulateEngine*> engineList;

    public slots:
        void interrupt();

    private slots:
        void finalizePopulating(bool result);

    signals:
        void populatingFinished();
        void populatingSuccessful();
        void populatingFailed();
        void orderWorkerToInterrupt();
};

#define POPULATE_MANAGER SQLITESTUDIO->getPopulateManager()

#endif // POPULATEMANAGER_H
