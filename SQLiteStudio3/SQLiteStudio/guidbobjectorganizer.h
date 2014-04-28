#ifndef GUIDBOBJECTORGANIZER_H
#define GUIDBOBJECTORGANIZER_H

#include "dbobjectorganizer.h"

class GuiDbObjectOrganizer : public DbObjectOrganizer
{
    public:
        GuiDbObjectOrganizer();
    private:
        bool confirmReferencedTables(const QStringList& tables);
        bool resolveNameConflict(QString& nameInConflict);
};

#endif // GUIDBOBJECTORGANIZER_H
