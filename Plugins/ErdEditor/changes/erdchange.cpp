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
    if (!skipSaveoints && !getTransactionId().isNull())
        ddl << savepointTpl.arg(getTransactionId());

    ddl += getChangeDdl();
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

ErdChange::Category ErdChange::getCategory() const
{
    return category;
}
