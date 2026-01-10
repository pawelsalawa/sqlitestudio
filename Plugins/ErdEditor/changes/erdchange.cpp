#include "erdchange.h"
#include "erdchangecomposite.h"
#include "common/global.h"
#include "common/utils.h"
#include <QUuid>

ErdChange::ErdChange(const QString& description, bool generateTransactionId) :
    description(description)
{
    if (generateTransactionId)
        transactionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QStringList ErdChange::getCachedChangeDdl()
{
    if (cachedDdl.isEmpty())
        cachedDdl = getChangeDdl();

    return cachedDdl;
}

QString ErdChange::getDescription() const
{
    return description;
}

bool ErdChange::isDdlChange()
{
    return !getCachedChangeDdl().isEmpty();
}

ErdChange* ErdChange::normalizeChanges(const QList<ErdChange*>& changes, const QString& compositeDescription)
{
    if (changes.isEmpty())
        return nullptr;

    if (changes.size() == 1)
        return changes.first();

    return new ErdChangeComposite(changes, compositeDescription);
}

QStringList ErdChange::toDdl(bool skipSaveoints)
{
    static_qstring(savepointTpl, "SAVEPOINT '%1'");

    QStringList ddl;
    if (!skipSaveoints && !getTransactionId().isNull())
        ddl << savepointTpl.arg(getTransactionId());

    ddl += getCachedChangeDdl();
    return ddl;
}

QStringList ErdChange::getUndoDdl()
{
    static_qstring(rollbackTpl, "ROLLBACK TO '%1'");

    if (getTransactionId().isNull())
        return QStringList();

    return {rollbackTpl.arg(getTransactionId())};
}

void ErdChange::applyRedo(ErdScene::SceneChangeApi& api)
{
    apply(api);
}

QString ErdChange::getTransactionId() const
{
    return transactionId;
}
