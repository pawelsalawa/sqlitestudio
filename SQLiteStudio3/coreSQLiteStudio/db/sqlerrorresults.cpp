#include "sqlerrorresults.h"
#include "common/unused.h"

SqlErrorResults::SqlErrorResults(int code, const QString& text)
{
    errText = text;
    errCode = code;
}

QString SqlErrorResults::getErrorText()
{
    return errText;
}

int SqlErrorResults::getErrorCode()
{
    return errCode;
}

QStringList SqlErrorResults::getColumnNames()
{
    return QStringList();
}

int SqlErrorResults::columnCount()
{
    return 0;
}

qint64 SqlErrorResults::rowsAffected()
{
    return 0;
}

SqlResultsRowPtr SqlErrorResults::nextInternal()
{
    return SqlResultsRowPtr();
}

bool SqlErrorResults::hasNextInternal()
{
    return false;
}

bool SqlErrorResults::execInternal(const QList<QVariant>& args)
{
    UNUSED(args);
    return false;
}

bool SqlErrorResults::execInternal(const QHash<QString, QVariant>& args)
{
    UNUSED(args);
    return false;
}
