#include "populatemanager.h"
#include "services/pluginmanager.h"
#include "plugins/populateplugin.h"
#include "services/notifymanager.h"
#include "populateworker.h"
#include "plugins/populatesequence.h"
#include "plugins/populaterandom.h"
#include "plugins/populaterandomtext.h"
#include "plugins/populateconstant.h"
#include "plugins/populatedictionary.h"
#include <QDebug>
#include <QThreadPool>

PopulateManager::PopulateManager(QObject *parent) :
    PluginServiceBase(parent)
{
    PLUGINS->loadBuiltInPlugin(new PopulateSequence());
    PLUGINS->loadBuiltInPlugin(new PopulateRandom());
    PLUGINS->loadBuiltInPlugin(new PopulateRandomText());
    PLUGINS->loadBuiltInPlugin(new PopulateConstant());
    PLUGINS->loadBuiltInPlugin(new PopulateDictionary());
}

void PopulateManager::populate(Db* db, const QString& table, const QHash<QString, PopulateEngine*>& engines, qint64 rows)
{
    if (workInProgress)
    {
        error();
        qCritical() << "Tried to call second populating process at the same time.";
        return;
    }

    if (!db->isOpen())
    {
        error();
        qCritical() << "Tried to populate table in closed database.";
        return;
    }

    workInProgress = true;

    columns.clear();
    engineList.clear();
    for (const QString& column : engines.keys())
    {
        columns << column;
        engineList << engines[column];
    }


    this->db = db;
    this->table = table;

    PopulateWorker* worker = new PopulateWorker(db, table, columns, engineList, rows);
    connect(worker, SIGNAL(finished(bool)), this, SLOT(finalizePopulating(bool)));
    connect(this, SIGNAL(orderWorkerToInterrupt()), worker, SLOT(interrupt()));

    QThreadPool::globalInstance()->start(worker);

}

void PopulateManager::error()
{
    emit populatingFinished();
    emit populatingFailed();
}

void PopulateManager::deleteEngines(const QList<PopulateEngine*>& engines)
{
    for (PopulateEngine* engine : engines)
        delete engine;
}

void PopulateManager::interrupt()
{
    emit orderWorkerToInterrupt();
}

void PopulateManager::finalizePopulating(bool result)
{
    workInProgress = false;

    emit populatingFinished();
    if (result)
    {
        notifyInfo(tr("Table '%1' populated successfully.").arg(table));
        emit populatingSuccessful();
    }
    else
        emit populatingFailed();
}
