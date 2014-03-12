#ifndef CLIUTILS_H
#define CLIUTILS_H

#include <QString>
#include <QVariant>

void initCliUtils();

int getCliColumns();
int getCliRows();

struct AsciiTree
{
    QList<AsciiTree> childs;
    QString label;
};

Q_DECLARE_METATYPE(AsciiTree)

QString toAsciiTree(const AsciiTree& tree);

#endif // CLIUTILS_H
