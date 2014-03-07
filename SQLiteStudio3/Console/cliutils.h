#ifndef CLIUTILS_H
#define CLIUTILS_H

#include "sortedhash.h"
#include <QString>
#include <QVariant>

void initCliUtils();

int getCliColumns();
int getCliRows();

typedef SortedHash<QString, QVariant> AsciiTree;
Q_DECLARE_METATYPE(AsciiTree)

QString toAsciiTree(const AsciiTree& tree);

#endif // CLIUTILS_H
