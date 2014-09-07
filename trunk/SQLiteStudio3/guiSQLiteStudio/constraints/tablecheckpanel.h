#ifndef TABLECHECKPANEL_H
#define TABLECHECKPANEL_H

#include "constraintcheckpanel.h"
#include "constraintpanel.h"
#include "guiSQLiteStudio_global.h"
#include <QWidget>

class GUI_API_EXPORT TableCheckPanel : public ConstraintCheckPanel
{
        Q_OBJECT

    public:
        explicit TableCheckPanel(QWidget *parent = 0);

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

#endif // TABLECHECKPANEL_H
