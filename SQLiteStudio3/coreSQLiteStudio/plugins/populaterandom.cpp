#include "populaterandom.h"
#include "services/populatemanager.h"
#include <QDateTime>
#include <QRandomGenerator>

PopulateRandom::PopulateRandom()
{
}

QString PopulateRandom::getTitle() const
{
    return tr("Random number");
}

PopulateEngine* PopulateRandom::createEngine()
{
    return new PopulateRandomEngine();
}

bool PopulateRandomEngine::beforePopulating(Db* db, const QString& table)
{
    Q_UNUSED(db);
    Q_UNUSED(table);
    randomGenerator = QRandomGenerator::securelySeeded();
    range = cfg.PopulateRandom.MaxValue.get() - cfg.PopulateRandom.MinValue.get() + 1;
    return (range > 0);
}

QVariant PopulateRandomEngine::nextValue(bool& nextValueError)
{
    Q_UNUSED(nextValueError);
    QString randValue = QString::number((randomGenerator.generate() % range) + cfg.PopulateRandom.MinValue.get());
    return (cfg.PopulateRandom.Prefix.get() + randValue + cfg.PopulateRandom.Suffix.get());
}

void PopulateRandomEngine::afterPopulating()
{
}

CfgMain* PopulateRandomEngine::getConfig()
{
    return &cfg;
}

QString PopulateRandomEngine::getPopulateConfigFormName() const
{
    return QStringLiteral("PopulateRandomConfig");
}

bool PopulateRandomEngine::validateOptions()
{
    bool valid = (cfg.PopulateRandom.MinValue.get() <= cfg.PopulateRandom.MaxValue.get());
    POPULATE_MANAGER->handleValidationFromPlugin(valid, cfg.PopulateRandom.MaxValue, QObject::tr("Maximum value cannot be less than minimum value."));
    return valid;
}
