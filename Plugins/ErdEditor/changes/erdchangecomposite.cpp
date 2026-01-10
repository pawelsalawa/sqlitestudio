#include "erdchangecomposite.h"

ErdChangeComposite::ErdChangeComposite(const QString& description) :
    ErdChange(description, true)
{
}

ErdChangeComposite::ErdChangeComposite(QList<ErdChange*> changes, const QString& description) :
    ErdChange(description, true), changes(changes)
{
}

void ErdChangeComposite::addChange(ErdChange* change)
{
    changes << change;
}

ErdChangeComposite &ErdChangeComposite::operator<<(ErdChange* change)
{
    changes << change;
    return *this;
}

ErdChangeComposite &ErdChangeComposite::operator+=(const QList<ErdChange*>& changeList)
{
    changes += changeList;
    return *this;
}

QStringList ErdChangeComposite::getChangeDdl()
{
    QStringList results;
    for (ErdChange*& chg : changes)
        results += chg->toDdl(true);

    return results;
}

QList<ErdChange*> ErdChangeComposite::getChanges() const
{
    return changes;
}

QStringList ErdChangeComposite::getUndoDdl()
{
    if (!changes.isEmpty())
        return changes.first()->getUndoDdl();

    return ErdChange::getUndoDdl();
}

QString ErdChangeComposite::getTransactionId() const
{
    if (!changes.isEmpty())
        return changes.first()->getTransactionId();

    return ErdChange::getTransactionId();
}

void ErdChangeComposite::apply(ErdScene::SceneChangeApi& api)
{
    for (auto&& singleChange : changes)
        singleChange->apply(api);
}

void ErdChangeComposite::applyUndo(ErdScene::SceneChangeApi& api)
{
    for (auto it = changes.crbegin(); it != changes.crend(); ++it) // reverse order
        (*it)->applyUndo(api);
}

void ErdChangeComposite::applyRedo(ErdScene::SceneChangeApi& api)
{
    for (auto&& singleChange : changes)
        singleChange->applyRedo(api);
}
