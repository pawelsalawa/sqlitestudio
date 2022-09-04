#include "dbpluginstdfilebase.h"
#include "common/unused.h"
#include "db/sqlquery.h"
#include <QFileInfo>

Db *DbPluginStdFileBase::getInstance(const QString &name, const QString &path, const QHash<QString, QVariant> &options, QString *errorMessage)
{
    UNUSED(errorMessage);

    Db* db = newInstance(name, path, options);

    if (!db->openForProbing())
    {
        if (errorMessage)
            *errorMessage = db->getErrorText();

        delete db;
        return nullptr;
    }

    SqlQueryPtr results = db->exec("SELECT * FROM sqlite_master");
    if (results->isError())
    {
        if (errorMessage)
            *errorMessage = db->getErrorText();

        delete db;
        return nullptr;
    }

    db->closeQuiet();
    return db;
}

QString DbPluginStdFileBase::generateDbName(const QVariant &baseValue)
{
    QFileInfo file(baseValue.toString());
    return file.completeBaseName();
}
