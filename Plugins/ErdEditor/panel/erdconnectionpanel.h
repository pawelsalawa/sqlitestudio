#ifndef ERDCONNECTIONPANEL_H
#define ERDCONNECTIONPANEL_H

#include <QWidget>
#include "common/extactioncontainer.h"
#include "erdpropertiespanel.h"
#include "parser/ast/sqlitecreatetable.h"

namespace Ui {
    class ErdConnectionPanel;
}

class ErdChange;
class Db;
class ErdEntity;
class ErdConnection;
class ConstraintPanel;
class ChainExecutor;

class ErdConnectionPanel : public QWidget, public ExtActionContainer, public ErdPropertiesPanel
{
        Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR_MAIN
        };

        ErdConnectionPanel(Db* db, ErdConnection* connection, QWidget *parent = nullptr);
        ~ErdConnectionPanel();

        QString getStartEntityTable() const;
        bool commitErdChange();
        void abortErdChange();

    protected:
        void createActions();
        void setupDefShortcuts();
        QToolBar *getToolBar(int toolbar) const;

    private:
        void init(ErdConnection *connection);
        void initColumnLevelFk(ErdConnection *connection);
        void initTableLevelFk(ErdConnection *connection);
        void createColumnLevelPanel();
        void createTableLevelPanel();

        Ui::ErdConnectionPanel *ui;
        Db* db = nullptr;
        ConstraintPanel* constraintPanel = nullptr;
        SqliteCreateTablePtr originalCreateTable;
        SqliteCreateTablePtr createTable;
        QString originalContent;
        ChainExecutor* ddlExecutor = nullptr;
        SqliteStatement* matchedFk = nullptr;
        SqliteCreateTable::Column* childColumnStmt = nullptr;
        QString originalReferencedTable;

    private slots:
        bool commit();
        void rollback();
        void validate();

    signals:
        void changeCreated(ErdChange* change);
};

#endif // ERDCONNECTIONPANEL_H
