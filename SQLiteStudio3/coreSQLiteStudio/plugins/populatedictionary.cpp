#include "populatedictionary.h"
#include "services/populatemanager.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

PopulateDictionary::PopulateDictionary()
{
}

QString PopulateDictionary::getTitle() const
{
    return tr("Dictionary", "dictionary populating plugin name");
}

PopulateEngine*PopulateDictionary::createEngine()
{
    return new PopulateDictionaryEngine();
}

bool PopulateDictionaryEngine::beforePopulating(Db* db, const QString& table)
{
    UNUSED(db);
    UNUSED(table);
    QFile file(cfg.PopulateDictionary.File.get());
    if (!file.open(QIODevice::ReadOnly))
    {
        notifyError(QObject::tr("Could not open dictionary file %1 for reading.").arg(cfg.PopulateDictionary.File.get()));
        return false;
    }
    QTextStream stream(&file);
    QString dataStr = stream.readAll();
    file.close();

    if (cfg.PopulateDictionary.Lines.get())
        dictionary = dataStr.split("\n");
    else
        dictionary = dataStr.split(QRegExp("\\s+"));

    if (dictionary.size() == 0)
        dictionary << QString();

    dictionaryPos = 0;
    dictionarySize = dictionary.size();
    if (cfg.PopulateDictionary.Random.get())
        qsrand(QDateTime::currentDateTime().toTime_t());

    return true;
}

QVariant PopulateDictionaryEngine::nextValue()
{
    if (cfg.PopulateDictionary.Random.get())
    {
        int r = qrand() % dictionarySize;
        return dictionary[r];
    }
    else
    {
        if (dictionaryPos >= dictionarySize)
            dictionaryPos = 0;

        return dictionary[dictionaryPos++];
    }
}

void PopulateDictionaryEngine::afterPopulating()
{
    dictionary.clear();
    dictionarySize = 0;
    dictionaryPos = 0;
}

CfgMain* PopulateDictionaryEngine::getConfig()
{
    return &cfg;
}

QString PopulateDictionaryEngine::getPopulateConfigFormName() const
{
    return QStringLiteral("PopulateDictionaryConfig");
}

bool PopulateDictionaryEngine::validateOptions()
{
    QFileInfo fi(cfg.PopulateDictionary.File.get());
    bool fileValid = fi.exists() && fi.isReadable() && !fi.isDir();
    POPULATE_MANAGER->handleValidationFromPlugin(fileValid, cfg.PopulateDictionary.File, QObject::tr("Dictionary file must exist and be readable."));

    return fileValid;
}
