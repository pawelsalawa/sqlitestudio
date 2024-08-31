#include "regexpimport.h"
#include "services/notifymanager.h"
#include "common/encodedtextstream.h"
#include "common/utils.h"
#include "services/importmanager.h"
#include "sqlitestudio.h"
#include <QRegularExpression>
#include <QFile>

RegExpImport::RegExpImport()
{
}

bool RegExpImport::init()
{
    SQLS_INIT_RESOURCE(regexpimport);
    return GenericPlugin::init();
}

void RegExpImport::deinit()
{
    SQLS_CLEANUP_RESOURCE(regexpimport);
}

QString RegExpImport::getDataSourceTypeName() const
{
    return "RegExp";
}

ImportManager::StandardConfigFlags RegExpImport::standardOptionsToEnable() const
{
    return ImportManager::CODEC|ImportManager::FILE_NAME;
}

QString RegExpImport::getFileFilter() const
{
    return tr("Text files (*.txt);;All files (*)");
}

bool RegExpImport::beforeImport(const ImportManager::StandardImportConfig& config)
{
    safe_delete(re);
    safe_delete(file);
    safe_delete(stream);
    groups.clear();
    buffer.clear();
    columns.clear();


    file = new QFile(config.inputFileName);
    if (!file->open(QFile::ReadOnly) || !file->isReadable())
    {
        notifyError(tr("Cannot read file %1").arg(config.inputFileName));
        safe_delete(file);
        return false;
    }

    stream = new EncodedTextStream(file);
    stream->setCodec(config.codec.toLatin1().data());


    static const QString intColTemplate = QStringLiteral("column%1");
    re = new QRegularExpression(cfg.RegExpImport.Pattern.get());
    QString colName;
    if (cfg.RegExpImport.GroupsMode.get() == "all")
    {
        for (int i = 1; i <= re->captureCount(); i++)
        {
            groups << i;
            colName = intColTemplate.arg(i);
            columns << generateUniqueName(colName, columns);
        }
    }
    else
    {
        QStringList entries = cfg.RegExpImport.CustomGroupList.get().split(QRegularExpression(",\\s*"));
        int i;
        bool ok;
        for (const QString& entry : entries)
        {
            i = entry.toInt(&ok);
            if (ok)
            {
                groups << i;
                colName = intColTemplate.arg(i);
            }
            else
            {
                groups << entry;
                colName = entry;
            }
            columns << generateUniqueName(colName, columns);
        }
    }
    return true;
}

void RegExpImport::afterImport()
{
    safe_delete(re);
    safe_delete(file);
    safe_delete(stream);
    buffer.clear();
    groups.clear();
}

QList<ImportPlugin::ColumnDefinition> RegExpImport::getColumns() const
{
    QList<ImportPlugin::ColumnDefinition> columnList;
    for (const QString& colName : columns)
        columnList << ImportPlugin::ColumnDefinition(colName, QString());

    return columnList;
}

QList<QVariant> RegExpImport::next()
{
    QRegularExpressionMatch match = re->match(buffer);
    QString line;
    while (!match.hasMatch() && !(line = stream->readLine()).isNull())
    {
        buffer += line;
        match = re->match(buffer);
    }

    if (!match.hasMatch())
        return QList<QVariant>();

    QList<QVariant> values;
    for (const QVariant& group : groups)
    {
        if (group.userType() == QMetaType::Int)
            values << match.captured(group.toInt());
        else
            values << match.captured(group.toString());
    }

    buffer = buffer.mid(match.capturedEnd());

    return values;
}

CfgMain* RegExpImport::getConfig()
{
    return &cfg;
}

QString RegExpImport::getImportConfigFormName() const
{
    return "RegExpImportConfig";
}

bool RegExpImport::validateOptions()
{
    QString reMsg;
    QString pattern = cfg.RegExpImport.Pattern.get();
    bool reOk = true;
    if (pattern.isEmpty())
    {
        reOk = false;
        reMsg = tr("Enter the regular expression pattern.");
    }

    QRegularExpression localRe(pattern);
    if (reOk)
    {
        reOk &= localRe.isValid();
        if (!reOk)
            reMsg = tr("Invalid pattern: %1").arg(localRe.errorString());
    }

    QString groupMsg;
    bool groupsOk = true;
    bool isCustom = (cfg.RegExpImport.GroupsMode.get() == "custom");
    if (isCustom && reOk)
    {
        QStringList entries = cfg.RegExpImport.CustomGroupList.get().split(QRegularExpression(",\\s*"));
        QStringList namedCaptureGroups = localRe.namedCaptureGroups();
        int captureCount = localRe.captureCount();
        int i;
        bool ok;
        for (const QString& entry : entries)
        {
            i = entry.toInt(&ok);
            if (ok)
            {
                if (i < 0 || i > captureCount)
                {
                    groupMsg = tr("Requested capture index %1 is out of range.").arg(i);
                    groupsOk = false;
                    break;
                }
            }
            else if (!namedCaptureGroups.contains(entry))
            {
                groupMsg = tr("<p>Requested capture group name '%1', but it's not defined in the pattern: <pre>%2</pre></p>")
                        .arg(entry, pattern.toHtmlEscaped());
                groupsOk = false;
                break;
            }
        }
    }

    IMPORT_MANAGER->handleValidationFromPlugin(reOk, cfg.RegExpImport.Pattern, reMsg);
    IMPORT_MANAGER->handleValidationFromPlugin(groupsOk, cfg.RegExpImport.CustomGroupList, groupMsg);
    IMPORT_MANAGER->updateVisibilityAndEnabled(cfg.RegExpImport.CustomGroupList, true, isCustom);

    return reOk && groupsOk;
}
