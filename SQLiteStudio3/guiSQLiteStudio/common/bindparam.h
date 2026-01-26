#ifndef BINDPARAM_H
#define BINDPARAM_H

#include "guiSQLiteStudio_global.h"
#include <QString>
#include <QVariant>

struct GUI_API_EXPORT BindParam
{
    int position = 0;
    QString originalName;
    QString newName;
    QVariant value;
};

#endif // BINDPARAM_H
