#include "populatedictionary.h"
#include "services/populatemanager.h"
#include "services/notifymanager.h"
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

bool PopulateDictionaryEngine::beforePopulating()
{
    file = new QFile(cfg.PopulateDictionary.File.get());
    if (!file->open(QIODevice::ReadOnly))
    {
        notifyError(QObject::tr("Could not open dictionary file %1 for reading.").arg(cfg.PopulateDictionary.File.get()));
        return false;
    }

    stream = new QTextStream(file);
    return true;
}

QVariant PopulateDictionaryEngine::nextValue()
{
    if (stream->atEnd())
        stream->seek(0);

    if (cfg.PopulateDictionary.Lines.get())
    {
        return stream->readLine();
    }
    else
    {
        QString word;
        *stream >> word;
        return word;
    }
}

void PopulateDictionaryEngine::afterPopulating()
{
    delete stream;
    delete file;
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
