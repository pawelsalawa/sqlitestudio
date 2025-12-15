#include "collections.h"

int operator|(const QStringList& list, const ListIndexOfStrOp& op)
{
    if (op.cs == Qt::CaseSensitive)
        return list.indexOf(op.value, op.from);

    int cnt = list.size();
    for (int i = op.from; i < cnt; i++)
        if (list[i].compare(op.value, op.cs) == 0)
            return i;

    return -1;
}
