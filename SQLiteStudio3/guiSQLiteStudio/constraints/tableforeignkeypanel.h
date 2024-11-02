#ifndef TABLEFOREIGNKEYPANEL_H
#define TABLEFOREIGNKEYPANEL_H

#include "constraintpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "guiSQLiteStudio_global.h"
#include "common/strhash.h"
#include <QStringListModel>
#include <QWidget>

namespace Ui {
    class TableForeignKeyPanel;
}

class QGridLayout;
class QSignalMapper;

class GUI_API_EXPORT TableForeignKeyPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit TableForeignKeyPanel(QWidget *parent = 0);
        ~TableForeignKeyPanel();

        bool validate();
        void setDb(Db* value);

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
        void handleFkTypeMatched(QWidget* indicatorParent, const QString& localColumn, const QString fkColumn);

        Ui::TableForeignKeyPanel *ui = nullptr;
        QGridLayout* columnsLayout = nullptr;
        int totalColumns = 0;
        QStringListModel fkColumnsModel;
        QSignalMapper* columnSignalMapping = nullptr;
        StrHash<StrHash<DataType>> fkTableTypesCache;

    private slots:
        void updateState();
        void updateColumnState(int rowIdx, bool tableSelected);
        void updateColumnState(int rowIdx);
        void updateFkColumns();
};

#endif // TABLEFOREIGNKEYPANEL_H
