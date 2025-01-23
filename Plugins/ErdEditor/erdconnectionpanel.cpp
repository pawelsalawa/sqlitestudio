#include "erdconnectionpanel.h"
#include "ui_erdconnectionpanel.h"
#include "db/db.h"
#include "erdentity.h"
#include "erdchangeentity.h"
#include "erdconnection.h"
#include "constraints/columnforeignkeypanel.h"
#include "constraints/tableforeignkeypanel.h"
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

void ErdConnectionPanel::init()
{
    ErdEntity* childEntity = connection->getStartEntity();
    ErdEntity* parentEntity = connection->getEndEntity();
    ui->childTableName->setText(childEntity->getTableName());

    ColumnForeignKeyPanel* fkPanel = new ColumnForeignKeyPanel();
    ui->fkPanelContainer->layout()->addWidget(fkPanel);

    SqliteCreateTable* childCreateTable = childEntity->getTableModel().data();
    SqliteCreateTable::Column* childColumnStmt = childCreateTable->columns[connection->getStartEntityRow() - 1]; // -1 to resect entity header

    SqliteCreateTable* parentCreateTable = parentEntity->getTableModel().data();
    SqliteCreateTable::Column* parentColumnStmt = parentCreateTable->columns[connection->getEndEntityRow() - 1]; // -1 to resect entity header

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
