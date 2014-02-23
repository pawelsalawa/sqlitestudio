#ifndef COLUMNCHECKPANEL_H
#define COLUMNCHECKPANEL_H

#include "constraintcheckpanel.h"
#include "constraintpanel.h"
#include <QWidget>

class ColumnCheckPanel : public ConstraintCheckPanel
{
        Q_OBJECT

    public:
        explicit ColumnCheckPanel(QWidget *parent = 0);

    protected:
        SqliteExpr* readExpr();
        QString readName();
        void storeType();
        SqliteConflictAlgo readConflictAlgo();
        void storeExpr(SqliteExpr* expr);
        void storeName(const QString& name);
        void storeConflictAlgo(SqliteConflictAlgo algo);
        SqliteCreateTable* getCreateTable();
};

#endif // COLUMNCHECKPANEL_H
