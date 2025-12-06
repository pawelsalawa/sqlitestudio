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
#include <QDebug>

ErdConnectionPanel::ErdConnectionPanel(Db* db, ErdConnection* connection, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ErdConnectionPanel),
    db(db),
    connection(connection)
{
    ui->setupUi(this);
    init();
}

ErdConnectionPanel::~ErdConnectionPanel()
{
    safe_delete(ddlExecutor);
    delete ui;
}

QString ErdConnectionPanel::getStartEntityTable() const
{
    return connection->getStartEntity()->getTableName();
}

bool ErdConnectionPanel::commitErdChange()
{
    return commit();
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

void ErdConnectionPanel::init()
{
    initActions();

    ddlExecutor = new ChainExecutor();

    ErdEntity* childEntity = connection->getStartEntity();
    originalCreateTable = childEntity->getTableModel();
    createTable.reset(childEntity->getTableModel()->typeClone<SqliteCreateTable>());

    if (connection->isCompoundConnection())
        initTableLevelFk();
    else
        initColumnLevelFk();
}

void ErdConnectionPanel::initColumnLevelFk()
{
    ui->tableLevelFkLabel->setVisible(false);

    ErdEntity* childEntity = connection->getStartEntity();
    ErdEntity* parentEntity = connection->getEndEntity();
    ui->childTableName->setText(childEntity->getTableName());

    ErdColumnFkPanel* fkPanel = new ErdColumnFkPanel();
    ui->fkPanelContainer->layout()->addWidget(fkPanel);

    SqliteCreateTable* childCreateTable = createTable.data();
    SqliteCreateTable::Column* childColumnStmt = childCreateTable->columns[connection->getStartEntityRow() - 1]; // -1 to respect entity header

    SqliteCreateTable* parentCreateTable = parentEntity->getTableModel().data();
    SqliteCreateTable::Column* parentColumnStmt = parentCreateTable->columns[connection->getEndEntityRow() - 1]; // -1 to respect entity header

    QString parentTableName = parentCreateTable->table.toLower();
    QString parentColumnName = parentColumnStmt->name;
    QList<SqliteCreateTable::Column::Constraint*> fkList = childColumnStmt->getConstraints(SqliteCreateTable::Column::Constraint::FOREIGN_KEY);
    SqliteCreateTable::Column::Constraint* matchedFk = nullptr;
    for (SqliteCreateTable::Column::Constraint* fk : fkList)
    {
        if (fk->foreignKey->foreignTable.toLower() == parentTableName && fk->foreignKey->getColumnNames().contains(parentColumnName, Qt::CaseInsensitive))
        {
            matchedFk = fk;
            break;
        }
    }

    if (!matchedFk)
    {
        qCritical() << "ErdConnectionPanel failed to match FK constraint!";
        return;
    }

    fkPanel->setDb(db);
    fkPanel->setCreateTableStmt(childCreateTable);
    fkPanel->setColumnStmt(childColumnStmt);
    fkPanel->setConstraint(matchedFk);
    constraintPanel = fkPanel;
}

void ErdConnectionPanel::initTableLevelFk()
{
    ui->bottomSpacer->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui->topLevelLayout->invalidate();

    ErdEntity* childEntity = connection->getStartEntity();
    ErdEntity* parentEntity = connection->getEndEntity();
    ui->childTableName->setText(childEntity->getTableName());

    ErdTableFkPanel* fkPanel = new ErdTableFkPanel();
    ui->fkPanelContainer->layout()->addWidget(fkPanel);

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
    QSet<QString> parentColumnNames = toSet(map<SqliteCreateTable::Column*, QString>(parentColumnStmts,
                [](auto stmt) {return stmt->name;}));
    QSet<QString> childColumnNames = toSet(map<SqliteCreateTable::Column*, QString>(childColumnStmts,
                [](auto stmt) {return stmt->name;}));

    QList<SqliteCreateTable::Constraint*> fkList = childCreateTable->getConstraints(SqliteCreateTable::Constraint::FOREIGN_KEY);
    SqliteCreateTable::Constraint* matchedFk = nullptr;
    for (SqliteCreateTable::Constraint* fk : fkList)
    {
        if (fk->foreignKey->foreignTable.toLower() != parentTableName)
            continue;

        QStringList fkChildColumnNames = map<SqliteIndexedColumn*, QString>(fk->indexedColumns, [](auto idxCol) {return idxCol->getColumnName();});
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

    fkPanel->setDb(db);
    fkPanel->setCreateTableStmt(childCreateTable);
    fkPanel->setConstraint(matchedFk);
    constraintPanel = fkPanel;
}

bool ErdConnectionPanel::commit()
{
    constraintPanel->storeDefinition();

    ErdChange* change = new ErdChangeEntity(db, originalCreateTable, createTable);

    ddlExecutor->setAsync(false);
    ddlExecutor->setDb(db);
    ddlExecutor->setQueries(change->toDdl());
    ddlExecutor->setDisableForeignKeys(true);
    ddlExecutor->setDisableObjectDropsDetection(true);
    ddlExecutor->exec();

    emit changeCreated(change);
    return true;
}

void ErdConnectionPanel::rollback()
{

}
