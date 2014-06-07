#include "populaterandomtext.h"
#include "common/utils.h"
#include "services/populatemanager.h"

PopulateRandomText::PopulateRandomText()
{
}

QString PopulateRandomText::getTitle() const
{
    return tr("Random text");
}

PopulateEngine* PopulateRandomText::createEngine()
{
    return new PopulateRandomTextEngine();
}

bool PopulateRandomTextEngine::beforePopulating()
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    range = cfg.PopulateRandomText.MaxLength.get() - cfg.PopulateRandomText.MinLength.get() + 1;

    chars = "";

    if (cfg.PopulateRandomText.UseCustomSets.get())
    {
        chars = cfg.PopulateRandomText.CustomCharacters.get();
    }
    else if (cfg.PopulateRandomText.IncludeBinary.get())
    {
        for (int i = 0; i < 256; i++)
            chars.append(QChar((char)i));
    }
    else
    {
        if (cfg.PopulateRandomText.IncludeAlpha.get())
            chars += QStringLiteral("abcdefghijklmnopqrstuvwxyz");

        if (cfg.PopulateRandomText.IncludeNumeric.get())
            chars += QStringLiteral("0123456789");

        if (cfg.PopulateRandomText.IncludeWhitespace.get())
            chars += QStringLiteral(" \t\n");
    }

    return !chars.isEmpty();
}

QVariant PopulateRandomTextEngine::nextValue()
{
    int lgt = (qrand() % range) + cfg.PopulateRandomText.MinLength.get();
    return randStr(lgt, chars);
}

void PopulateRandomTextEngine::afterPopulating()
{
}

CfgMain* PopulateRandomTextEngine::getConfig()
{
    return &cfg;
}

QString PopulateRandomTextEngine::getPopulateConfigFormName() const
{
    return QStringLiteral("PopulateRandomTextConfig");
}

bool PopulateRandomTextEngine::validateOptions()
{
    bool rangeValid = (cfg.PopulateRandomText.MinLength.get() <= cfg.PopulateRandomText.MaxLength.get());
    POPULATE_MANAGER->handleValidationFromPlugin(rangeValid, cfg.PopulateRandomText.MaxLength, QObject::tr("Maximum length cannot be less than minimum length."));

    bool useCustom = cfg.PopulateRandomText.UseCustomSets.get();
    bool useBinary = cfg.PopulateRandomText.IncludeBinary.get();
    POPULATE_MANAGER->updateVisibilityAndEnabled(cfg.PopulateRandomText.IncludeAlpha, true, !useCustom && !useBinary);
    POPULATE_MANAGER->updateVisibilityAndEnabled(cfg.PopulateRandomText.IncludeNumeric, true, !useCustom && !useBinary);
    POPULATE_MANAGER->updateVisibilityAndEnabled(cfg.PopulateRandomText.IncludeWhitespace, true, !useCustom && !useBinary);
    POPULATE_MANAGER->updateVisibilityAndEnabled(cfg.PopulateRandomText.IncludeBinary, true, !useCustom);
    POPULATE_MANAGER->updateVisibilityAndEnabled(cfg.PopulateRandomText.CustomCharacters, true, useCustom);

    bool customValid = !useCustom || !cfg.PopulateRandomText.CustomCharacters.get().isEmpty();
    POPULATE_MANAGER->handleValidationFromPlugin(customValid, cfg.PopulateRandomText.CustomCharacters, QObject::tr("Custom character set cannot be empty."));

    return rangeValid && customValid;
}
