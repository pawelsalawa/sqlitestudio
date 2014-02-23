#ifndef COLUMNFOREIGNKEYPANEL_H
#define COLUMNFOREIGNKEYPANEL_H

#include "constraintpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QStringListModel>
#include <QWidget>

namespace Ui {
    class ColumnForeignKeyPanel;
}

class QGridLayout;
class QSignalMapper;

class ColumnForeignKeyPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit ColumnForeignKeyPanel(QWidget *parent = 0);
        ~ColumnForeignKeyPanel();

        bool validate();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();

    private:
        void init();
        void readConstraint();
        void readTables();
        void readCondition(SqliteForeignKey::Condition* condition);
        void storeCondition(SqliteForeignKey::Condition::Action action, const QString& reaction);
        void storeMatchCondition(const QString& reaction);

        Ui::ColumnForeignKeyPanel *ui;
        QStringListModel fkColumnsModel;

    private slots:
        void updateState();
        void updateFkColumns();
};

#endif // COLUMNFOREIGNKEYPANEL_H
