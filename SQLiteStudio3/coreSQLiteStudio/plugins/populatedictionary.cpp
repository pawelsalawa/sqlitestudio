#include "populatedictionary.h"
#include "services/populatemanager.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QRegularExpression>

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
        dictionary = dataStr.split(QRegularExpression("(\r\n|\n|\r)"));
    else
        dictionary = dataStr.split(QRegularExpression("\\s+"));

    if (dictionary.size() == 0)
        dictionary << QString();

    dictionaryPos = 0;
    dictionarySize = dictionary.size();
    if (cfg.PopulateDictionary.Random.get())
        QRandomGenerator::system()->seed(QDateTime::currentDateTime().toSecsSinceEpoch());

    return true;
}

QVariant PopulateDictionaryEngine::nextValue(bool& nextValueError)
{
    UNUSED(nextValueError);
    if (cfg.PopulateDictionary.Random.get())
    {
        int r = QRandomGenerator::system()->generate() % dictionarySize;
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
