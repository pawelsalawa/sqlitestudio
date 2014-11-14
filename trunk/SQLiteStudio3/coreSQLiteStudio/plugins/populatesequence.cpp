#include "populatesequence.h"
#include "common/global.h"
#include "services/populatemanager.h"
#include "common/unused.h"
#include <QVariant>

PopulateSequence::PopulateSequence()
{
}

QString PopulateSequence::getTitle() const
{
    return tr("Sequence");
}

PopulateEngine* PopulateSequence::createEngine()
{
    return new PopulateSequenceEngine();
}

bool PopulateSequenceEngine::beforePopulating(Db* db, const QString& table)
{
    UNUSED(db);
    UNUSED(table);
    seq = cfg.PopulateSequence.StartValue.get();
    step = cfg.PopulateSequence.Step.get();
    return true;
}

QVariant PopulateSequenceEngine::nextValue(bool& nextValueError)
{
    UNUSED(nextValueError);
    return seq += step;
}

void PopulateSequenceEngine::afterPopulating()
{
}

CfgMain* PopulateSequenceEngine::getConfig()
{
    return &cfg;
}

QString PopulateSequenceEngine::getPopulateConfigFormName() const
{
    return QStringLiteral("PopulateSequenceConfig");
}

bool PopulateSequenceEngine::validateOptions()
{
    return true;
}
