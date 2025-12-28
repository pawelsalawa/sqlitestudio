#include "erdtablewindow.h"
#include "scene/erdentity.h"
#include "ui_tablewindow.h"
#include "changes/erdchangeentity.h"
#include "changes/erdchangenewentity.h"

ErdTableWindow::ErdTableWindow(Db* db, ErdEntity* entity, QWidget* parent)
    : TableWindow(parent, db, QString(), entity->getTableName(), entity->isExistingTable()),
      entity(entity)
{
    ui->dbCombo->setEnabled(false);
    ui->dbCombo->setVisible(false);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->dataTab), false);

    ui->structureToolBar->removeAction(actionMap[REFRESH_STRUCTURE]);
    ui->structureToolBar->removeAction(separatorAfterAction[REFRESH_STRUCTURE]);

    QString commitText = tr("Apply changes to diagram", "ERD editor");
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

QString ErdTableWindow::getQuitUncommittedConfirmMessage() const
{
    // TODO
    return "";
}

bool ErdTableWindow::commitErdChange()
{
    return commitStructure(true);
}

void ErdTableWindow::abortErdChange()
{
    rollbackStructure();
}

bool ErdTableWindow::resolveCreateTableStatement()
{
    createTable = SqliteCreateTablePtr::create(*entity->getTableModel());
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

bool ErdTableWindow::commitStructure(bool skipWarning)
{
    if (!isModified())
        return true;

    if (skipWarning && createTable->columns.isEmpty() && !entity->isExistingTable())
    {
        emit editedEntityShouldBeDeleted(entity);
        return true;
    }

    return TableWindow::commitStructure(skipWarning);
}

void ErdTableWindow::rollbackStructure()
{
    TableWindow::rollbackStructure();
    entity->modelUpdated();
}

void ErdTableWindow::executeStructureChanges()
{
    ErdChange* change;
    if (entity->isExistingTable())
    {
        QString desc = tr("Update entity \"%1\".").arg(createTable->table);
        change = new ErdChangeEntity(db, originalCreateTable, createTable, desc);
    }
    else
    {
        QString desc = tr("Create entity \"%1\".").arg(createTable->table);
        change = new ErdChangeNewEntity(db, originalCreateTable->table, createTable, desc);
    }

    structureExecutor->setTransaction(false);
    structureExecutor->setAsync(false);
    structureExecutor->setDb(db);
    structureExecutor->setQueries(change->toDdl());
    structureExecutor->setRollbackOnErrorTo(change->getTransactionId());
    structureExecutor->setDisableForeignKeys(true);
    structureExecutor->setDisableObjectDropsDetection(true);
    structureExecutor->exec();

    if (structureExecutor->getSuccessfulExecution())
        emit changeCreated(change);
    else
        qWarning() << "Failed to execute entity changes on in-memory database:" << structureExecutor->getErrorsMessages();
}
