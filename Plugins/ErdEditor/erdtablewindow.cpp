#include "erdtablewindow.h"
#include "erdentity.h"
#include "ui_tablewindow.h"
#include "erdchangeentity.h"

ErdTableWindow::ErdTableWindow(Db* db, ErdEntity* entity, QWidget* parent)
    : TableWindow(parent, db, QString(), entity->getTableNameForEditing(), entity->isExistingTable()),
      entity(entity)
{
    ui->dbCombo->setEnabled(false);
    ui->dbCombo->setVisible(false);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->dataTab), false);

    ui->structureToolBar->removeAction(actionMap[REFRESH_STRUCTURE]);
    ui->structureToolBar->removeAction(separatorAfterAction[REFRESH_STRUCTURE]);

    QString commitText = tr("Apply changes to diagram entity", "ERD editor");
    QString cancelText = tr("Abort changes", "ERD editor");
    actionMap[COMMIT_STRUCTURE]->setText(commitText);
    actionMap[COMMIT_STRUCTURE]->setToolTip(commitText);
    actionMap[ROLLBACK_STRUCTURE]->setText(cancelText);
    actionMap[ROLLBACK_STRUCTURE]->setToolTip(cancelText);

    QList<QAction*> structActions = ui->structureToolBar->actions();
    int removeFrom = structActions.indexOf(separatorAfterAction[ADD_TRIGGER_STRUCT]);
    for (QAction*& act : structActions.mid(removeFrom))
        ui->structureToolBar->removeAction(act);

    initDbAndTable();
    updateAfterInit();
}

ErdTableWindow::~ErdTableWindow()
{
}

bool ErdTableWindow::resolveCreateTableStatement()
{
    createTable = entity->getPendingTableModel().isNull() ?
                SqliteCreateTablePtr::create(*entity->getTableModel()) :
                entity->getPendingTableModel();

    return true;
}

bool ErdTableWindow::resolveOriginalCreateTableStatement()
{
    originalCreateTable = SqliteCreateTablePtr::create(*entity->getTableModel());
    return true;
}

void ErdTableWindow::applyInitialTab()
{
    ui->tabWidget->setCurrentIndex(getStructureTabIdx());
}

void ErdTableWindow::defineCurrentContextDb()
{
    // No-op, as we use in-memory db for this window and it's not on the dropdown list.
    // The dropdown list is hidden anyway.
}

void ErdTableWindow::changesSuccessfullyCommitted()
{
    modifyingThisTable = false;
    updateWindowAfterStructureChanged();
}

void ErdTableWindow::storePendingTableModel()
{
    if (!isModified())
        return;

    entity->setPendingTableModel(createTable);
    entity->modelUpdated();
}

void ErdTableWindow::rollbackStructure()
{
    TableWindow::rollbackStructure();
    entity->setPendingTableModel(SqliteCreateTablePtr());
    entity->modelUpdated();
}

void ErdTableWindow::executeStructureChanges()
{
    ErdChangeEntity* change = new ErdChangeEntity(entity, db, originalCreateTable, createTable);

    structureExecutor->setAsync(false);
    structureExecutor->setDb(db);
    structureExecutor->setQueries(change->toDdl());
    structureExecutor->setDisableForeignKeys(true);
    structureExecutor->setDisableObjectDropsDetection(true);
    structureExecutor->exec();

    emit changeCreated(change);
}
