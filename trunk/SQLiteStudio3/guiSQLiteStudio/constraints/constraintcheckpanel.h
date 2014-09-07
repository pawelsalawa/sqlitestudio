#ifndef CONSTRAINTCHECKPANEL_H
#define CONSTRAINTCHECKPANEL_H

#include "constraintpanel.h"
#include "parser/ast/sqliteconflictalgo.h"
#include "guiSQLiteStudio_global.h"
#include <QWidget>

namespace Ui {
    class ConstraintCheckPanel;
}

class GUI_API_EXPORT ConstraintCheckPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit ConstraintCheckPanel(QWidget *parent = 0);
        ~ConstraintCheckPanel();

        bool validate();
        bool validateOnly();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();
        virtual SqliteExpr* readExpr() = 0;
        virtual QString readName() = 0;
        virtual void storeType() = 0;
        virtual SqliteConflictAlgo readConflictAlgo() = 0;
        virtual void storeExpr(SqliteExpr* expr) = 0;
        virtual void storeName(const QString& name) = 0;
        virtual void storeConflictAlgo(SqliteConflictAlgo algo) = 0;
        virtual SqliteCreateTable* getCreateTable() = 0;

    private:
        void init();
        void readConstraint();
        void updateVirtualSql();
        SqliteExprPtr parseExpression(const QString& sql);

        Ui::ConstraintCheckPanel *ui;

    private slots:
        void updateState();
};

#endif // CONSTRAINTCHECKPANEL_H
