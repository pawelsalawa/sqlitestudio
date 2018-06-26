#ifndef BINDPARAM_H
#define BINDPARAM_H

#include <QString>
#include <QVariant>

struct BindParam
{
    int position = 0;
    QString originalName;
    QString newName;
    QVariant value;
};

#endif // BINDPARAM_H
