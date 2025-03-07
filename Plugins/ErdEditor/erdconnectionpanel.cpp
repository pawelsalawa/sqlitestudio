#include "erdconnectionpanel.h"
#include "ui_erdconnectionpanel.h"
#include "db/db.h"
#include "erdentity.h"
#include "erdchangeentity.h"
#include "erdconnection.h"
#include "erdcolumnfkpanel.h"
#include "erdtablefkpanel.h"
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
    delete ui;
}

QString ErdConnectionPanel::getStartEntityTable() const
{
    return connection->getStartEntity()->getTableName();
}

void ErdConnectionPanel::init()
{
    if (connection->isCompoundConnection())
    {
        initTableLevelFk();
        ui->bottomSpacer->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
        ui->topLevelLayout->invalidate();
    }
    else
        initColumnLevelFk();
}

void ErdConnectionPanel::initColumnLevelFk()
{
    ErdEntity* childEntity = connection->getStartEntity();
    ErdEntity* parentEntity = connection->getEndEntity();
    ui->childTableName->setText(childEntity->getTableName());

    ErdColumnFkPanel* fkPanel = new ErdColumnFkPanel();
    ui->fkPanelContainer->layout()->addWidget(fkPanel);

    SqliteCreateTable* childCreateTable = childEntity->getTableModel().data();
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
}

void ErdConnectionPanel::initTableLevelFk()
{
    ErdEntity* childEntity = connection->getStartEntity();
    ErdEntity* parentEntity = connection->getEndEntity();
    ui->childTableName->setText(childEntity->getTableName());

    ErdTableFkPanel* fkPanel = new ErdTableFkPanel();
    ui->fkPanelContainer->layout()->addWidget(fkPanel);

    SqliteCreateTable* childCreateTable = childEntity->getTableModel().data();
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
}
