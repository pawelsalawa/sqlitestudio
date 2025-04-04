#include "erdchange.h"
#include "common/global.h"
#include <QUuid>

ErdChange::ErdChange(Category category, bool generateTransactionId) :
    category(category)
{
    if (generateTransactionId)
        transactionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
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

QString ErdChange::getTransactionId() const
{
    return transactionId;
}

ErdChange::Category ErdChange::getCategory() const
{
    return category;
}
