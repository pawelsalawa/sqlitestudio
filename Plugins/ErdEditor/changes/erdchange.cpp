#include "erdchange.h"
#include "common/global.h"
#include "common/utils.h"
#include <QUuid>

ErdChange::ErdChange(Category category, const QString& description, bool generateTransactionId) :
    category(category), description(description)
{
    if (generateTransactionId)
        transactionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString ErdChange::getDescription() const
{
    return description;
}

QStringList ErdChange::toDdl(bool skipSaveoints)
{
    static_qstring(savepointTpl, "SAVEPOINT '%1'");

    QStringList ddl;
    if (!skipSaveoints && !transactionId.isNull())
        ddl << savepointTpl.arg(transactionId);

    ddl += getChangeDdl();
    return ddl;
}

QStringList ErdChange::getUndoDdl()
{
    static_qstring(rollbackTpl, "ROLLBACK TO '%1'");

    if (transactionId.isNull())
        return QStringList();

    return {rollbackTpl.arg(transactionId)};
}

QStringList ErdChange::getEntitiesToRefreshAfterUndo() const
{
    // Instead of converting list->set->list, we do manual uniqueness processing,
    // because order of returned names do matter.
    QSet<QString> set;
    return provideUndoEntitiesToRefresh() | FILTER(name,
    {
        QString lower = name.toLower();
        if (set.contains(lower))
            return false;

        set << lower;
        return true;
    });
}

QString ErdChange::getTransactionId() const
{
    return transactionId;
}

ErdChange::Category ErdChange::getCategory() const
{
    return category;
}
