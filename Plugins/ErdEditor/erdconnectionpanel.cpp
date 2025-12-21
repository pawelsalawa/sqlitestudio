#include "erdconnectionpanel.h"
#include "ui_erdconnectionpanel.h"
#include "db/db.h"
#include "db/chainexecutor.h"
#include "erdentity.h"
#include "erdchangeentity.h"
#include "erdconnection.h"
#include "erdcolumnfkpanel.h"
#include "erdtablefkpanel.h"
#include "common/unused.h"
#include "iconmanager.h"
#include "services/notifymanager.h"
#include <QDebug>

ErdConnectionPanel::ErdConnectionPanel(Db* db, ErdConnection* connection, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ErdConnectionPanel),
    db(db)
{
    ui->setupUi(this);
    init(connection);
}

ErdConnectionPanel::~ErdConnectionPanel()
{
    safe_delete(ddlExecutor);
    delete ui;
}

QString ErdConnectionPanel::getStartEntityTable() const
{
    return createTable->table;
}

bool ErdConnectionPanel::commitErdChange()
{
    return commit();
}

void ErdConnectionPanel::abortErdChange()
{
    rollback();
}

void ErdConnectionPanel::createActions()
{
    createAction(COMMIT, ICONS.COMMIT, tr("Apply changes to diagram", "ERD editor"), this, SLOT(commit()), ui->toolBar, this);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Abort changes", "ERD editor"), this, SLOT(rollback()), ui->toolBar, this);
}

void ErdConnectionPanel::setupDefShortcuts()
{
}

QToolBar *ErdConnectionPanel::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolBar;
}

void ErdConnectionPanel::init(ErdConnection* connection)
{
    initActions();

    ddlExecutor = new ChainExecutor();
    ddlExecutor->setAsync(false);
    ddlExecutor->setDb(db);
    ddlExecutor->setDisableForeignKeys(true);
    ddlExecutor->setDisableObjectDropsDetection(true);

    originalReferencedTable = connection->getEndEntity()->getTableName();

    ErdEntity* childEntity = connection->getStartEntity();
    originalCreateTable = childEntity->getTableModel();
    createTable.reset(originalCreateTable->typeClone<SqliteCreateTable>());
    originalContent = originalCreateTable->produceTokens().detokenize();

    if (connection->isTableLevelFk())
        initTableLevelFk(connection);
    else
        initColumnLevelFk(connection);

    actionMap[COMMIT]->setEnabled(false);
    actionMap[ROLLBACK]->setEnabled(false);
}

void ErdConnectionPanel::initColumnLevelFk(ErdConnection* connection)
{
    ui->tableLevelFkLabel->setVisible(false);

    ErdEntity* childEntity = connection->getStartEntity();
    ErdEntity* parentEntity = connection->getEndEntity();
    ui->childTableName->setText(childEntity->getTableName());

    SqliteCreateTable* childCreateTable = createTable.data();
    childColumnStmt = childCreateTable->columns[connection->getStartEntityRow() - 1]; // -1 to respect entity header
    ui->childColumnName->setText(childColumnStmt->name);

    SqliteCreateTable* parentCreateTable = parentEntity->getTableModel().data();
    SqliteCreateTable::Column* parentColumnStmt = parentCreateTable->columns[connection->getEndEntityRow() - 1]; // -1 to respect entity header

    QString parentTableName = parentCreateTable->table.toLower();
    QString parentColumnName = parentColumnStmt->name;
    QList<SqliteCreateTable::Column::Constraint*> fkList = childColumnStmt->getConstraints(SqliteCreateTable::Column::Constraint::FOREIGN_KEY);
    matchedFk = nullptr;
    for (SqliteCreateTable::Column::Constraint*& fk : fkList)
    {
        if (fk->foreignKey->foreignTable.toLower() == parentTableName && fk->foreignKey->getColumnNames().contains(parentColumnName, Qt::CaseInsensitive))
        {
            matchedFk = fk;
            break;
        }
    }
    createColumnLevelPanel();
}

void ErdConnectionPanel::initTableLevelFk(ErdConnection* connection)
{
    ui->childColumnName->hide();
    ui->childColumnTitleLabel->hide();
    ui->bottomSpacer->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui->topLevelLayout->invalidate();

    ErdEntity* childEntity = connection->getStartEntity();
    ErdEntity* parentEntity = connection->getEndEntity();
    ui->childTableName->setText(childEntity->getTableName());

    SqliteCreateTable* childCreateTable = createTable.data();
    QList<SqliteCreateTable::Column*> childColumnStmts;
    childColumnStmts << childCreateTable->columns[connection->getStartEntityRow() - 1]; // -1 to respect entity header
    for (ErdConnection* assocConn : connection->getAssociatedConnections())
        childColumnStmts << childCreateTable->columns[assocConn->getStartEntityRow() - 1];

    SqliteCreateTable* parentCreateTable = parentEntity->getTableModel().data();
    QList<SqliteCreateTable::Column*> parentColumnStmts;
    parentColumnStmts << parentCreateTable->columns[connection->getEndEntityRow() - 1]; // -1 to respect entity header
    for (ErdConnection* assocConn : connection->getAssociatedConnections())
        parentColumnStmts << parentCreateTable->columns[assocConn->getEndEntityRow() - 1];

    QString parentTableName = parentCreateTable->table.toLower();
    QSet<QString> parentColumnNames = toSet(parentColumnStmts | MAP(stmt, {return stmt->name;}));
    QSet<QString> childColumnNames = toSet(childColumnStmts | MAP(stmt, {return stmt->name;}));

    QList<SqliteCreateTable::Constraint*> fkList = childCreateTable->getConstraints(SqliteCreateTable::Constraint::FOREIGN_KEY);
    matchedFk = nullptr;
    for (SqliteCreateTable::Constraint*& fk : fkList)
    {
        if (fk->foreignKey->foreignTable.toLower() != parentTableName)
            continue;

        QStringList fkChildColumnNames = fk->indexedColumns | MAP(idxCol, {return idxCol->getColumnName();});
        if (childColumnNames != toSet(fkChildColumnNames))
            continue;

        QStringList fkParentColumnNames = fk->foreignKey->getColumnNames();
        if (fkParentColumnNames.isEmpty())
            fkParentColumnNames = fkChildColumnNames;

        if (parentColumnNames != toSet(fkParentColumnNames))
            continue;

        matchedFk = fk;
        break;
    }
    createTableLevelPanel();
}

void ErdConnectionPanel::createColumnLevelPanel()
{
    ErdColumnFkPanel* fkPanel = new ErdColumnFkPanel();
    ui->fkPanelContainer->layout()->addWidget(fkPanel);
    if (!matchedFk)
    {
        qCritical() << "ErdConnectionPanel failed to match FK constraint!";
        return;
    }
    fkPanel->setDb(db);
    fkPanel->setCreateTableStmt(createTable.data());
    fkPanel->setColumnStmt(childColumnStmt);
    fkPanel->setConstraint(matchedFk);
    constraintPanel = fkPanel;
    connect(constraintPanel, SIGNAL(updateValidation()), this, SLOT(validate()));
}

void ErdConnectionPanel::createTableLevelPanel()
{
    ErdTableFkPanel* fkPanel = new ErdTableFkPanel();
    ui->fkPanelContainer->layout()->addWidget(fkPanel);
    fkPanel->setDb(db);
    fkPanel->setCreateTableStmt(createTable.data());
    fkPanel->setConstraint(matchedFk);
    constraintPanel = fkPanel;
    connect(constraintPanel, SIGNAL(updateValidation()), this, SLOT(validate()));
}

bool ErdConnectionPanel::commit()
{
    if (!actionMap[COMMIT]->isEnabled())
    {
        if (!actionMap[ROLLBACK]->isEnabled())
        {
            // Not modified at all.
            return true;
        }
        // TODO ask
        return false;
    }

    constraintPanel->storeDefinition();

    QString newContent = createTable->produceTokens().detokenize();
    if (originalContent == newContent)
        return true;

    QString changeDesc = (originalReferencedTable == createTable->table) ?
              tr("Modify relationship between \"%1\" and \"%2\".")
                .arg(originalCreateTable->table, originalReferencedTable) :
              tr("Modify relationship between \"%1\" and \"%2\" - change target to \"%3\".")
                .arg(originalCreateTable->table, originalReferencedTable, createTable->table);

    ErdChange* change = new ErdChangeEntity(db, originalCreateTable, createTable, changeDesc);
    ddlExecutor->setQueries(change->toDdl());
    ddlExecutor->exec();
    if (!ddlExecutor->getSuccessfulExecution())
    {
        delete change;
        notifyError(tr("Failed to execute DDL required for relation modification. Details: %1")
                    .arg(ddlExecutor->getErrorsMessages().join("; ")));
        return false;
    }

    actionMap[COMMIT]->setEnabled(false);
    actionMap[ROLLBACK]->setEnabled(false);
    emit changeCreated(change);
    return true;
}

void ErdConnectionPanel::rollback()
{
    ui->fkPanelContainer->layout()->removeWidget(constraintPanel);
    safe_delete(constraintPanel);

    if (ui->tableLevelFkLabel->isVisible())
        createTableLevelPanel();
    else
        createColumnLevelPanel();

    actionMap[COMMIT]->setEnabled(false);
    actionMap[ROLLBACK]->setEnabled(false);
}

void ErdConnectionPanel::validate()
{
    actionMap[COMMIT]->setEnabled(constraintPanel->validate());
    actionMap[ROLLBACK]->setEnabled(true);
}
