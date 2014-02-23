#ifndef TABLEFOREIGNKEYPANEL_H
#define TABLEFOREIGNKEYPANEL_H

#include "constraintpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include <QStringListModel>
#include <QWidget>

namespace Ui {
    class TableForeignKeyPanel;
}

class QGridLayout;
class QSignalMapper;

class TableForeignKeyPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit TableForeignKeyPanel(QWidget *parent = 0);
        ~TableForeignKeyPanel();

        bool validate();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();

    private:
        void init();
        void buildColumns();
        void buildColumn(SqliteCreateTable::Column* column, int row);
        void readConstraint();
        void readTables();
        void readCondition(SqliteForeignKey::Condition* condition);
        int getColumnIndex(const QString& colName);
        void storeCondition(SqliteForeignKey::Condition::Action action, const QString& reaction);
        void storeMatchCondition(const QString& reaction);

        Ui::TableForeignKeyPanel *ui;
        QGridLayout* columnsLayout;
        int totalColumns = 0;
        QStringListModel fkColumnsModel;
        QSignalMapper* columnSignalMapping;

    private slots:
        void updateState();
        void updateColumnState(int rowIdx, bool tableSelected);
        void updateColumnState(int rowIdx);
        void updateFkColumns();
};

#endif // TABLEFOREIGNKEYPANEL_H
