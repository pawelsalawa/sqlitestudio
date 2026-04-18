#include "dbadiantuminstance.h"
#include "dbadiantum.h"
#include "db/db.h"
#include "common/utils_sql.h"
#include "adiantum_vfs.h"
#include <QFileInfo>
#include <QUrl>
#include <QRegularExpression>
#include <QDebug>

const char* AdiantumDriver::label = "adiantum";

int AdiantumDriver::open_v2(const char* filename, handle** ppDb, int flags, const char* zVfs)
{
    // For Adiantum, we always use the "adiantum" VFS
    // The key must be registered before calling this
    return sqlite3_open_v2(filename, ppDb, flags, zVfs);
}

DbAdiantumInstance::DbAdiantumInstance(const QString& name, const QString& path,
                                       const QHash<QString, QVariant>& connOptions)
    : AbstractDb3<AdiantumDriver>(name, path, connOptions)
{
}

Db* DbAdiantumInstance::clone() const
{
    // Get connection options - use const_cast to call non-const method from const method
    QHash<QString, QVariant> opts = const_cast<DbAdiantumInstance*>(this)->getConnectionOptions();
    return new DbAdiantumInstance(getName(), getPath(), opts);
}

QString DbAdiantumInstance::getTypeClassName() const
{
    return "DbAdiantumInstance";
}

QString DbAdiantumInstance::getTypeLabel() const
{
    return tr("Adiantum");
}

bool DbAdiantumInstance::openInternal()
{
    resetError();

    const bool isPlain = getConnectionOptions()[DbAdiantum::PLAIN_OPT].toBool();

    // Register the key BEFORE opening the database
    if (!isPlain) {
        const QString hexKey = getConnectionOptions()[DbAdiantum::HEXKEY_OPT].toString().trimmed();
        if (!hexKey.isEmpty()) {
            // Validate hex key format
            QRegularExpression hexKeyRe("^[0-9a-fA-F]{64}$");
            if (!hexKeyRe.match(hexKey).hasMatch()) {
                setError(AdiantumDriver::ERROR, QObject::tr("Invalid hex key format. Must be 64 hexadecimal characters."));
                return false;
            }

            // Decode hex key to raw bytes
            QByteArray rawKey = QByteArray::fromHex(hexKey.toLatin1());
            if (rawKey.size() != 32) {
                setError(AdiantumDriver::ERROR, QObject::tr("Invalid hex key: decoded to %1 bytes, expected 32").arg(rawKey.size()));
                return false;
            }

            // Get canonical path for key registration
            QString canonicalPath = QFileInfo(getPath()).canonicalFilePath();
            if (canonicalPath.isEmpty()) {
                canonicalPath = getPath();
            }

            // Register the key with the VFS
            AdiantumVFS::registerMainDbKey(canonicalPath.toStdString(),
                                          reinterpret_cast<const uint8_t*>(rawKey.constData()));
        }
    }

    // Open the database using the Adiantum VFS
    AdiantumDriver::handle* h = nullptr;
    int res = AdiantumDriver::open_v2(getPath().toUtf8().constData(), &h,
                                       AdiantumDriver::OPEN_READWRITE | AdiantumDriver::OPEN_CREATE,
                                       isPlain ? nullptr : "adiantum");
    if (res != AdiantumDriver::OK) {
        setError(res, QObject::tr("Could not open database: %1").arg(QString::fromUtf8(sqlite3_errmsg(h))));
        if (h) {
            AdiantumDriver::close(h);
        }
        return false;
    }
    setDbHandle(h);
    AdiantumDriver::db_config(h, AdiantumDriver::DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL);
    return true;
}

void DbAdiantumInstance::initAfterOpen()
{
    SqlQueryPtr res;

    const bool isPlain = getConnectionOptions()[DbAdiantum::PLAIN_OPT].toBool();

    if (!isPlain) {
        // Disable mmap to prevent SQLite from bypassing the VFS for page I/O.
        res = exec("PRAGMA mmap_size = 0;", Db::Flag::NO_LOCK);
        // Note: no PRAGMA hexkey call here — the key is pre-registered with
        // AdiantumVFS before sqlite3_open_v2 so the VFS already owns it.
    }

    // Execute custom PRAGMAs
    const QString pragmas = getConnectionOptions()[DbAdiantum::PRAGMAS_OPT].toString();
    for (const QString& pragma : quickSplitQueries(pragmas)) {
        QString sql = removeComments(pragma).trimmed();
        if (sql.isEmpty()) continue;
        res = exec(sql, Db::Flag::NO_LOCK);
        if (res->isError())
            qWarning() << "Adiantum: PRAGMA failed:" << pragma << ":" << res->getErrorText();
    }

    // Call base class initialization
    AbstractDb3<AdiantumDriver>::initAfterOpen();
}

QString DbAdiantumInstance::getAttachSql(Db* otherDb, const QString& generatedAttachName)
{
    auto* otherAdiantum = dynamic_cast<DbAdiantumInstance*>(otherDb);
    const QString attachName = wrapObjIfNeeded(generatedAttachName);
    const QString otherPath = otherDb->getPath();

    if (!otherAdiantum) {
        return QStringLiteral("ATTACH %1 AS %2;").arg(escapeString(otherPath), attachName);
    }

    const auto& opts = otherAdiantum->getConnectionOptions();
    const bool otherPlain = opts[DbAdiantum::PLAIN_OPT].toBool();

    if (!otherPlain) {
        const QString otherHex = opts[DbAdiantum::HEXKEY_OPT].toString().trimmed();
        if (!otherHex.isEmpty()) {
            QByteArray rawKey = QByteArray::fromHex(otherHex.toLatin1());
            if (rawKey.size() == 32) {
                QString canonical = QFileInfo(otherPath).canonicalFilePath();
                if (canonical.isEmpty()) canonical = otherPath;
                AdiantumVFS::registerMainDbKey(canonical.toStdString(),
                                              reinterpret_cast<const uint8_t*>(rawKey.constData()));
            }
        }
        // URI filename must be percent-encoded and quoted as an SQL string literal.
        const QString uri = QStringLiteral("file:") + QString::fromUtf8(QUrl::toPercentEncoding(otherPath, "/"))
                            + QStringLiteral("?vfs=adiantum");
        return QStringLiteral("ATTACH %1 AS %2;").arg(escapeString(uri), attachName);
    }

    const QString uri = QStringLiteral("file:") + QString::fromUtf8(QUrl::toPercentEncoding(otherPath, "/"));
    return QStringLiteral("ATTACH %1 AS %2;").arg(escapeString(uri), attachName);
}
