#include "services/config.h"

CFG_DEFINE(Core)

static const QString DB_FILE_NAME = QStringLiteral("settings3");
static QString MASTER_CONFIG_FILE = QString();

Config::~Config()
{
}

void Config::setMasterConfigFile(const QString &path)
{
    MASTER_CONFIG_FILE = path;
}

QString Config::getMasterConfigFile()
{
    return MASTER_CONFIG_FILE;
}
