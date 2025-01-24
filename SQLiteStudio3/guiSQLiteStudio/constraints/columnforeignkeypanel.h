#ifndef COLUMNFOREIGNKEYPANEL_H
#define COLUMNFOREIGNKEYPANEL_H

#include "constraintpanel.h"
#include "parser/ast/sqlitecreatetable.h"
#include "guiSQLiteStudio_global.h"
#include <QStringListModel>
#include <QWidget>

namespace Ui {
    class ColumnForeignKeyPanel;
}

class QGridLayout;
class QSignalMapper;

class GUI_API_EXPORT ColumnForeignKeyPanel : public ConstraintPanel
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

        void init();
        void readConstraint();
        void readTables();
        void readCondition(SqliteForeignKey::Condition* condition);
        void storeCondition(SqliteForeignKey::Condition::Action action, const QString& reaction);
        void storeMatchCondition(const QString& reaction);

        Ui::ColumnForeignKeyPanel *ui = nullptr;
        QStringListModel fkColumnsModel;

    protected slots:
        void updateState();
        void updateFkColumns();
};

#endif // COLUMNFOREIGNKEYPANEL_H
