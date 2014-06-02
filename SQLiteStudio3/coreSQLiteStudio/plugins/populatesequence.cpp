#include "populatesequence.h"
#include "common/global.h"
#include "services/populatemanager.h"
#include <QVariant>

PopulateSequence::PopulateSequence()
{
}

PopulateEngine* PopulateSequence::createEngine()
{
    return new PopulateSequenceEngine();
}

bool PopulateSequenceEngine::beforePopulating()
{
    seq = cfg.PopulateSequence.StartValue.get();
    step = cfg.PopulateSequence.Step.get();
    return true;
}

QVariant PopulateSequenceEngine::nextValue()
{
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
