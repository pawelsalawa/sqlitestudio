#include "populateconstant.h"

PopulateConstant::PopulateConstant()
{
}

QString PopulateConstant::getTitle() const
{
    return tr("Constant", "populate constant plugin name");
}

PopulateEngine*PopulateConstant::createEngine()
{
    return new PopulateConstantEngine();
}

bool PopulateConstantEngine::beforePopulating(Db* db, const QString& table)
{
    Q_UNUSED(db);
    Q_UNUSED(table);
    return true;
}

QVariant PopulateConstantEngine::nextValue(bool& nextValueError)
{
    Q_UNUSED(nextValueError);
    return cfg.PopulateConstant.Value.get();
}

void PopulateConstantEngine::afterPopulating()
{
}

CfgMain*PopulateConstantEngine::getConfig()
{
    return &cfg;
}

QString PopulateConstantEngine::getPopulateConfigFormName() const
{
    return QStringLiteral("PopulateConstantConfig");
}

bool PopulateConstantEngine::validateOptions()
{
    return true;
}
