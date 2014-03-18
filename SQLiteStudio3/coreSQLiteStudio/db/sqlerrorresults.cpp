#include "sqlerrorresults.h"

SqlErrorResults::SqlErrorResults(int code, const QString& text)
{
    errText = text;
    errCode = code;
}

SqlResultsRowPtr SqlErrorResults::next()
{
    return SqlResultsRowPtr();
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
