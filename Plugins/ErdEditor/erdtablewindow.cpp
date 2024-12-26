#include "erdtablewindow.h"
#include "erdentity.h"
#include "ui_tablewindow.h"

ErdTableWindow::ErdTableWindow(Db* db, ErdEntity* entity, QWidget* parent)
    : TableWindow(parent, db, QString(), entity->getTableName(), entity->isExistingTable()),
      entity(entity)
{
    ui->dbCombo->setEnabled(false);
    ui->dbCombo->setVisible(false);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->dataTab), false);

    ui->structureToolBar->removeAction(actionMap[REFRESH_STRUCTURE]);
    ui->structureToolBar->removeAction(separatorAfterAction[REFRESH_STRUCTURE]);

    QString commitText = tr("Apply changes to diagram entity", "ERD editor");
    QString cancelText = tr("Cancel changes", "ERD editor");
    actionMap[COMMIT_STRUCTURE]->setText(commitText);
    actionMap[COMMIT_STRUCTURE]->setToolTip(commitText);
    actionMap[ROLLBACK_STRUCTURE]->setText(cancelText);
    actionMap[ROLLBACK_STRUCTURE]->setToolTip(cancelText);

    QList<QAction*> structActions = ui->structureToolBar->actions();
    int removeFrom = structActions.indexOf(separatorAfterAction[ADD_TRIGGER_STRUCT]);
    for (QAction*& act : structActions.mid(removeFrom))
        ui->structureToolBar->removeAction(act);

    initDbAndTable();
}

ErdTableWindow::~ErdTableWindow()
{
}

bool ErdTableWindow::resolveCreateTableStatement()
{
    createTable = entity->getTableModel();
    return true;
}

bool ErdTableWindow::resolveOriginalCreateTableStatement()
{
    if (entity->getOldTableModel().isNull())
        return TableWindow::resolveOriginalCreateTableStatement();

    originalCreateTable = entity->getOldTableModel();
    return true;
}

void ErdTableWindow::applyInitialTab()
{
    ui->tabWidget->setCurrentIndex(getStructureTabIdx());
}

void ErdTableWindow::executeStructureChanges()
{
    entity->setModifiedTableModel(originalCreateTable, createTable);
}
