#ifndef TABLEWINDOW_H
#define TABLEWINDOW_H

#include "db/db.h"
#include "mdichild.h"
#include "common/extactioncontainer.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreateindex.h"
#include "dialogs/constraintdialog.h"
#include "db/chainexecutor.h"
#include <QPointer>

class TableModifier;
class SqlTableModel;
class ExtLineEdit;
class IntValidator;
class QLabel;
class TableStructureModel;
class TableConstraintsModel;
class QProgressBar;
class WidgetCover;
class SqliteSyntaxHighlighter;
class ConstraintTabModel;

namespace Ui {
    class TableWindow;
}

class TableWindow : public MdiChild, public ExtActionContainer
{
        Q_OBJECT

    public:
        enum Action
        {
            // Structure tab
            REFRESH_STRUCTURE,
            COMMIT_STRUCTURE,
            ROLLBACK_STRUCTURE,
            ADD_COLUMN,
            EDIT_COLUMN,
            DEL_COLUMN,
            MOVE_COLUMN_UP,
            MOVE_COLUMN_DOWN,
            ADD_TABLE_CONSTRAINT,
            EDIT_TABLE_CONSTRAINT,
            DEL_TABLE_CONSTRAINT,
            ADD_TABLE_PK,
            ADD_TABLE_FK,
            ADD_TABLE_UNIQUE,
            ADD_TABLE_CHECK,
            MOVE_CONSTRAINT_UP,
            MOVE_CONSTRAINT_DOWN,
            EXPORT,
            IMPORT,
            POPULATE,
            CREATE_SIMILAR,
            // Indexes tab
            REFRESH_INDEXES,
            ADD_INDEX,
            EDIT_INDEX,
            DEL_INDEX,
            // Triggers tab
            REFRESH_TRIGGERS,
            ADD_TRIGGER,
            EDIT_TRIGGER,
            DEL_TRIGGER,
            // All tabs
            NEXT_TAB,
            PREV_TAB
        };

        explicit TableWindow(QWidget *parent = 0);
        TableWindow(Db* db, QWidget *parent = 0);
        TableWindow(const TableWindow& win);
        TableWindow(QWidget *parent, Db* db, const QString& database, const QString& table);
        ~TableWindow();

        static void staticInit();

        QString getTable() const;
        Db* getDb() const;
        bool handleInitialFocus();

    protected:
        void changeEvent(QEvent *e);
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();
        bool restoreSessionNextTime();

    private:
        void init();
        void newTable();
        void parseDdl();
        void initDbAndTable();
        void setupCoverWidget();
        void createStructureActions();
        void createDataGridActions();
        void createDataFormActions();
        void createIndexActions();
        void createTriggerActions();
        void editColumn(const QModelIndex& idx);
        void delColumn(const QModelIndex& idx);
        void editConstraint(const QModelIndex& idx);
        void delConstraint(const QModelIndex& idx);
        void executeStructureChanges();
        QModelIndex structureCurrentIndex() const;
        void addConstraint(ConstraintDialog::Constraint mode);
        bool validate();
        bool isModified() const;
        TokenList indexColumnTokens(SqliteCreateIndexPtr index);
        QString getCurrentIndex() const;
        QString getCurrentTrigger() const;
        void applyInitialTab();

        int newTableWindowNum = 1;

        Db* db = nullptr;
        QString database;
        QString table;
        Ui::TableWindow *ui;
        SqlTableModel* dataModel;
        bool dataLoaded = false;
        bool existingTable = true;
        bool blankNameWarningDisplayed = false;
        SqliteCreateTablePtr createTable;
        SqliteCreateTablePtr originalCreateTable;
        TableStructureModel* structureModel = nullptr;
        TableConstraintsModel* structureConstraintsModel = nullptr;
        ConstraintTabModel* constraintTabModel = nullptr;
        WidgetCover* widgetCover;
        ChainExecutor* structureExecutor = nullptr;
        TableModifier* tableModifier = nullptr;

    private slots:
        void executionSuccessful();
        void executionFailed(const QString& errorText);
        void dbClosed();
        void checkIfTableDeleted(const QString& database, const QString& object, DbObjectType type);
        void refreshStructure();
        void commitStructure();
        void changesSuccessfullyCommited();
        void changesFailedToCommit(int errorCode, const QString& errorText);
        void rollbackStructure();
        void addColumn();
        void editColumn();
        void delColumn();
        void moveColumnUp();
        void moveColumnDown();
        void addConstraint();
        void editConstraint();
        void delConstraint();
        void moveConstraintUp();
        void moveConstraintDown();
        void addPk();
        void addFk();
        void addUnique();
        void addCheck();
        void exportTable();
        void importTable();
        void populateTable();
        void createSimilarTable();
        void tabChanged(int newTab);
        void updateStructureToolbarState();
        void updateStructureCommitState();
        void updateTableConstraintsToolbarState();
        void updateDdlTab();
        void updateNewTableState();
        void updateBlankNameWarningState();
        void on_structureView_doubleClicked(const QModelIndex &index);
        void on_tableConstraintsView_doubleClicked(const QModelIndex &index);
        void nameChanged();
        void withOutRowIdChanged();
        void addIndex();
        void editIndex();
        void delIndex();
        void addTrigger();
        void editTrigger();
        void delTrigger();
        void updateIndexesState();
        void updateTriggersState();
        void nextTab();
        void prevTab();

    public slots:
        void updateIndexes();
        void updateTriggers();
        void editColumn(const QString& columnName);

    signals:
        void modifyStatusChanged();
};

#endif // TABLEWINDOW_H
