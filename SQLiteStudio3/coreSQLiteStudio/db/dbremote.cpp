#include "dbremote.h"

DbRemote::DbRemote()
{
}

DbRemote::~DbRemote()
{
}

bool DbRemote::openQuiet()
{
    return false;
}

bool DbRemote::closeQuiet()
{
    return false;
}

bool DbRemote::isOpenInternal()
{
    return false;
}

bool DbRemote::init()
{
    return false;
}

Db* DbRemote::getInstance(const QString& path, const QString &options)
{
    return nullptr;
}

SqlResultsPtr DbRemote::execInternal(const QString &query, const QList<QVariant> &args, bool singleCell)
{
    return SqlResultsPtr();
}

SqlResultsPtr DbRemote::execInternal(const QString &query, const QMap<QString, QVariant> &args, bool singleCell)
{
    return SqlResultsPtr();
}

QString DbRemote::getTypeLabel()
{
    return "REMOTE";
}

QString DbRemote::getErrorText()
{
    return QString::null;
}

int DbRemote::getErrorCode()
{
    return 0;
}
