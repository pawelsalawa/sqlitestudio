#ifndef TRIGGERDIALOG_H
#define TRIGGERDIALOG_H

#include "db/db.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include <QDialog>

namespace Ui {
    class TriggerDialog;
}

class TriggerDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit TriggerDialog(Db* db, QWidget *parent = 0);
        ~TriggerDialog();

        void setParentTable(const QString& name);
        void setParentView(const QString& name);
        void setTrigger(const QString& name);

    protected:
        void changeEvent(QEvent *e);

    private:
        void init();
        void initTrigger();
        void parseDdl();
        void readTrigger();
        void setupVirtualSqls();
        void readColumns();
        QString getTargetObjectName() const;
        void rebuildTrigger();

        QString originalTriggerName;
        QString trigger;
        QString table;
        QString view;
        Db* db = nullptr;
        bool forTable = true;
        bool existingTrigger = false;
        QStringList targetColumns;
        QStringList selectedColumns;
        QString ddl;
        SqliteCreateTriggerPtr createTrigger;
        Ui::TriggerDialog *ui;

    private slots:
        void updateState();
        void updateValidation();
        void showColumnsDialog();
        void updateDdlTab(int tabIdx);
        void tableChanged(const QString& newValue);

    public slots:
        void accept();
};

#endif // TRIGGERDIALOG_H
