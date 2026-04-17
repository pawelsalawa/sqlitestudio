#include "dbadiantum.h"
#include "dbadiantuminstance.h"
#include "common/utils_sql.h"
#include "adiantum_vfs.h"

DbAdiantum::DbAdiantum()
{
}

QString DbAdiantum::getLabel() const
{
    return tr("SQLite3 Adiantum VFS");
}

QList<DbPluginOption> DbAdiantum::getOptionsList() const
{
    QList<DbPluginOption> opts;

    // Hex Key option
    DbPluginOption optKey;
    optKey.type = DbPluginOption::PASSWORD;
    optKey.key = HEXKEY_OPT;
    optKey.label = tr("Hex Key (64 hex characters)");
    optKey.toolTip = tr("64-character hexadecimal encryption key. "
                        "Leave empty (and check Plain mode) to connect to an unencrypted database.");
    optKey.placeholderText = tr("00000000...00000000 (64 chars)");
    opts << optKey;

    // Plain mode option
    DbPluginOption optPlain;
    optPlain.type = DbPluginOption::BOOL;
    optPlain.key = PLAIN_OPT;
    optPlain.label = tr("Plain text mode (no encryption)");
    optPlain.toolTip = tr("Enable to create or connect to unencrypted database. "
                          "Bypasses the Adiantum VFS entirely.");
    optPlain.defaultValue = false;
    opts << optPlain;

    // Custom PRAGMAs option
    DbPluginOption optPragmas;
    optPragmas.type = DbPluginOption::SQL;
    optPragmas.key = PRAGMAS_OPT;
    optPragmas.label = tr("Custom PRAGMAs (optional)");
    optPragmas.defaultValue = "-- Recommended for the Adiantum VFS:\n"
                               "--PRAGMA journal_mode = WAL;\n"
                               "--PRAGMA busy_timeout = 5000;";
    optPragmas.toolTip = tr("PRAGMA statements executed after the database is opened. "
                            "Use them to opt into WAL, busy_timeout, etc.");
    opts << optPragmas;

    return opts;
}

bool DbAdiantum::checkIfDbServedByPlugin(Db* db) const
{
    Q_UNUSED(db);
    return false;
}

bool DbAdiantum::init()
{
    // Initialize the Adiantum VFS when the plugin loads
    AdiantumVFS::initialize();
    return true;
}

void DbAdiantum::deinit()
{
    // VFS cleanup if needed
}

Db* DbAdiantum::newInstance(const QString& name, const QString& path, const QHash<QString, QVariant>& options)
{
    return new DbAdiantumInstance(name, path, options);
}

const char* DbAdiantum::HEXKEY_OPT = "hexkey";
const char* DbAdiantum::PLAIN_OPT = "plain";
const char* DbAdiantum::PRAGMAS_OPT = "pragmas";
