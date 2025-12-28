#include "erdwindow.h"
#include "icon.h"
#include "iconmanager.h"
#include "ui_erdwindow.h"
#include "scene/erdscene.h"
#include "common/unused.h"
#include "services/dbmanager.h"
#include "mdiwindow.h"
#include "style.h"
#include "erdeditorplugin.h"
#include "services/config.h"
#include "scene/erdentity.h"
#include "panel/erdtablewindow.h"
#include "changes/erdchange.h"
#include "changes/erdchangeregistry.h"
#include "changes/erdchangeentity.h"
#include "db/sqlquery.h"
#include "db/sqlresultsrow.h"
#include "changes/erdchangecomposite.h"
#include "changes/erdchangedeleteconnection.h"
#include "changes/erdchangedeleteentity.h"
#include "changes/erdchangenewentity.h"
#include "changes/erdchangeregistrydialog.h"
#include "scene/erdconnection.h"
#include "panel/erdconnectionpanel.h"
#include "tablemodifier.h"
#include <QDebug>
#include <QMdiSubWindow>
#include <QActionGroup>
#include <QShortcut>
#include <QGraphicsOpacityEffect>
#include <QToolButton>
#include <QMessageBox>

Icon* ErdWindow::cursorAddTableIcon = nullptr;
Icon* ErdWindow::cursorFkIcon = nullptr;
Icon* ErdWindow::windowIcon = nullptr;
Icon* ErdWindow::fdpIcon = nullptr;
Icon* ErdWindow::neatoIcon = nullptr;
Icon* ErdWindow::lineCurvyIcon = nullptr;
Icon* ErdWindow::lineStraightIcon = nullptr;
Icon* ErdWindow::lineSquareIcon = nullptr;

ErdWindow::ErdWindow() :
    ui(new Ui::ErdWindow)
{
    init();
}

ErdWindow::ErdWindow(QWidget *parent, Db* db) :
    MdiChild(parent),
    ui(new Ui::ErdWindow), db(db)
{
    init();
}

ErdWindow::ErdWindow(const ErdWindow& other) :
    MdiChild(other.parentWidget()),
    ui(new Ui::ErdWindow), db(other.db)
{
    init();
}

ErdWindow::~ErdWindow()
{
    disconnect(scene, &QGraphicsScene::selectionChanged, this, &ErdWindow::itemSelectionChanged);
    delete ui;
}

void ErdWindow::staticInit()
{
    qRegisterMetaType<ErdWindow>("ErdWindow");

    cursorAddTableIcon = new Icon("ERD_CURSOR_TABLE_ADD", "cursor_table_add");
    cursorFkIcon = new Icon("ERD_CURSOR_FK", "cursor_fk");
    windowIcon = new Icon("ERD_EDITOR", "erdeditor");
    fdpIcon = new Icon("ERDLAYOUT_FDP", "erdlayout_fdp");
    neatoIcon = new Icon("ERDLAYOUT_NEATO", "erdlayout_neato");
    lineStraightIcon = new Icon("ERDEDITOR_LINE_STRAIGHT", "erdeditor_line_straight");
    lineCurvyIcon = new Icon("ERDEDITOR_LINE_CURVY", "erdeditor_line_curvy");
    lineSquareIcon = new Icon("ERDEDITOR_LINE_SQUARE", "erdeditor_line_square");
}

void ErdWindow::staticCleanup()
{
    delete lineStraightIcon;
    delete lineCurvyIcon;
    delete lineSquareIcon;
    delete windowIcon;
    delete fdpIcon;
    delete neatoIcon;
    delete cursorAddTableIcon;
    delete cursorFkIcon;
}

void ErdWindow::init()
{
    ui->setupUi(this);

    for (auto icon : {cursorAddTableIcon, cursorFkIcon, windowIcon, fdpIcon, neatoIcon,
                    lineStraightIcon, lineCurvyIcon, lineSquareIcon})
        icon->load();

#ifdef Q_OS_MACOS
    ui->bottomLabel->setText(ui->bottomLabel->text().replace("Ctrl+F", "Cmd+F"));
#endif

    ui->splitter->setSizes({400, 200});
    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 1);

    noSideWidgetContents = ui->noContentWidget;
    noSideWidgetEffect = new QGraphicsOpacityEffect(noSideWidgetContents);
    noSideWidgetEffect->setOpacity(0.5);
    noSideWidgetContents->setGraphicsEffect(noSideWidgetEffect);

    ErdArrowItem::Type arrowType = (ErdArrowItem::Type)CFG_ERD.Erd.ArrowType.get();

    scene = new ErdScene(arrowType, this);
    connect(scene, &ErdScene::changeReceived, this, &ErdWindow::handleCreatedChange, Qt::QueuedConnection);
    connect(scene, &ErdScene::sidePanelAbortRequested, this, &ErdWindow::abortSidePanel);
    connect(scene, &ErdScene::sidePanelRefreshRequested, this, &ErdWindow::refreshSidePanel);
    ui->view->setScene(scene);

    initActions();

    connect(STYLE, &Style::paletteChanged, this, &ErdWindow::uiPaletteChanged);
    connect(scene, &QGraphicsScene::selectionChanged, this, &ErdWindow::itemSelectionChanged);
    connect(ui->view, &ErdView::draftConnectionRemoved, this, &ErdWindow::handleDraftConnectionRemoved, Qt::QueuedConnection);
    connect(ui->view, &ErdView::tableInsertionAborted, this, &ErdWindow::handleTableInsertionAborted, Qt::QueuedConnection);
    connect(ui->view, &ErdView::newEntityPositionPicked, this, &ErdWindow::createNewEntityAt);

    changeRegistry = new ErdChangeRegistry(this);
    connect(changeRegistry, &ErdChangeRegistry::effectiveChangeCountUpdated, this, &ErdWindow::updateToolbarState);

    parseAndRestore();
    updateState();
}

void ErdWindow::createActions()
{
    createAction(CANCEL_CURRENT, tr("Cancels ongoing action", "ERD editor"), this, SLOT(cancelCurrentAction()), this);

    QActionGroup* lineGroup = new QActionGroup(ui->toolBar);
    lineGroup->setExclusive(true);
    actionMap[LINE_STRAIGHT] = new QAction(*lineStraightIcon, tr("Use straight line"), lineGroup);
    actionMap[LINE_CURVY] = new QAction(*lineCurvyIcon, tr("Use curvy line"), lineGroup);
    actionMap[LINE_SQUARE] = new QAction(*lineSquareIcon, tr("Use square line"), lineGroup);

    actionMap[LINE_STRAIGHT]->setCheckable(true);
    actionMap[LINE_CURVY]->setCheckable(true);
    actionMap[LINE_SQUARE]->setCheckable(true);
    updateArrowTypeButtons();

    connect(actionMap[LINE_STRAIGHT], &QAction::triggered, this, &ErdWindow::useStraightLine);
    connect(actionMap[LINE_CURVY], &QAction::triggered, this, &ErdWindow::useCurvyLine);
    connect(actionMap[LINE_SQUARE], &QAction::triggered, this, &ErdWindow::useSquareLine);

    createAction(RELOAD, ICONS.RELOAD, tr("Reload schema", "ERD editor"), this, SLOT(reloadSchema()), ui->toolBar);
    createAction(COMMIT, ICONS.COMMIT, tr("Commit all pending changes", "ERD editor"), this, SLOT(commitPendingChanges()), ui->toolBar);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback all pending changes", "ERD editor"), this, SLOT(rollbackPendingChanges()), ui->toolBar);

    createAction(UNDO, ICONS.ACT_UNDO, tr("Undo", "ERD editor"), this, SLOT(undo()), ui->toolBar, ui->view);
    createAction(REDO, ICONS.ACT_REDO, tr("Redo", "ERD editor"), this, SLOT(redo()), ui->toolBar, ui->view);

    ui->toolBar->addSeparator();
    changeCountLabel = new QToolButton(this);
    QFont bold = changeCountLabel->font();
    bold.setBold(true);
    changeCountLabel->setFont(bold);
    changeCountLabel->setText(QString::asprintf(CHANGE_COUNT_DIGITS, 0));
    changeCountLabel->setToolTip(tr("The number of changes pending for commit. Click to see details."));
    connect(changeCountLabel, &QToolButton::clicked, this, &ErdWindow::showChangeRegistry);
    ui->toolBar->addWidget(changeCountLabel);

    ui->toolBar->addAction(actionMap[UNDO]);
    ui->toolBar->addAction(actionMap[REDO]);
    ui->toolBar->addSeparator();
    createAction(NEW_TABLE, ICONS.TABLE_ADD, tr("Create a table (%1)").arg("T"), this, SLOT(newTableToggled(bool)), ui->toolBar, ui->view);
    createAction(ADD_CONNECTION, ICONS.CONSTRAINT_FOREIGN_KEY, tr("Add a foreign key (%1)").arg("F"), this, SLOT(addConnectionToggled(bool)), ui->toolBar, ui->view);
    ui->toolBar->addAction(actionMap[ADD_CONNECTION]);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(actionMap[LINE_STRAIGHT]);
    ui->toolBar->addAction(actionMap[LINE_CURVY]);
    ui->toolBar->addAction(actionMap[LINE_SQUARE]);
    ui->toolBar->addSeparator();
    createAction(ARRANGE_FDP, *fdpIcon, tr("Arrange entities using Force-Directed Placement approach"), scene, SLOT(arrangeEntitiesFdp()), ui->toolBar);
    createAction(ARRANGE_NEATO, *neatoIcon, tr("Arrange entities using Spring Model approach"), scene, SLOT(arrangeEntitiesNeato()), ui->toolBar);
}

void ErdWindow::setupDefShortcuts()
{
    new QShortcut(QKeySequence::Cancel, this, SLOT(cancelCurrentAction()), SLOT(cancelCurrentAction()), Qt::WidgetWithChildrenShortcut);

    actionMap[LINE_STRAIGHT]->setShortcut(Qt::CTRL|Qt::Key_1);
    actionMap[LINE_CURVY]->setShortcut(Qt::CTRL|Qt::Key_2);
    actionMap[LINE_SQUARE]->setShortcut(Qt::CTRL|Qt::Key_3);
    actionMap[UNDO]->setShortcut(QKeySequence::Undo);
    actionMap[REDO]->setShortcut(QKeySequence::Redo);
    actionMap[NEW_TABLE]->setShortcut(Qt::Key_T);
    actionMap[ADD_CONNECTION]->setShortcut(Qt::Key_R);

    setShortcutContext({UNDO, REDO}, Qt::WidgetWithChildrenShortcut);
}

void ErdWindow::checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState)
{
    if (!oldState.testFlag(Qt::WindowActive) && newState.testFlag(Qt::WindowActive))
        ui->view->setFocus();
}

void ErdWindow::uiPaletteChanged()
{
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    scene->update();
}

void ErdWindow::useStraightLine()
{
    applyArrowType(ErdArrowItem::STRAIGHT);
}

void ErdWindow::useStraightLineHotKey()
{
    actionMap[LINE_STRAIGHT]->trigger();
}

void ErdWindow::useCurvyLine()
{
    applyArrowType(ErdArrowItem::CURVY);
}

void ErdWindow::useCurvyLineHotKey()
{
    actionMap[LINE_CURVY]->trigger();
}

void ErdWindow::useSquareLine()
{
    applyArrowType(ErdArrowItem::SQUARE);
}

void ErdWindow::useSquareLineHotKey()
{
    actionMap[LINE_SQUARE]->trigger();
}

void ErdWindow::cancelCurrentAction()
{
    if (ui->view->isDraftingConnection() || ui->view->isPlacingNewEntity())
    {
        ui->view->popOperatingMode();
    }
    else if (currentSideWidget)
    {
        abortSidePanel();
        scene->clearSelection();
    }
}

void ErdWindow::newTableToggled(bool enable)
{
    if (enable && !storeCurrentSidePanelModifications())
        return;

    if (enable)
        ui->view->insertNewEntity();
    else if (ui->view->isPlacingNewEntity())
        ui->view->popOperatingMode();
}

void ErdWindow::addConnectionToggled(bool enable)
{
    if (enable && !storeCurrentSidePanelModifications())
        return;

    ui->view->setDraftingConnectionMode(enable);
}

void ErdWindow::handleDraftConnectionRemoved()
{
    actionMap[ADD_CONNECTION]->setChecked(false);
}

void ErdWindow::handleTableInsertionAborted()
{
    actionMap[NEW_TABLE]->setChecked(false);
}

void ErdWindow::createNewEntityAt(const QPointF& pos)
{
    SqliteCreateTable* tableModel = new SqliteCreateTable();
    tableModel->table = tr("table name", "ERD editor");

    ErdEntity* entity = new ErdEntity(tableModel);
    entity->setExistingTable(false);
    scene->placeNewEntity(entity, pos);

    focusItem(entity);
    entity->editName();
}

void ErdWindow::itemSelectionChanged()
{
    if (ignoreSelectionChangeEvents)
        return;

    QString sidePanelEntityName = getCurrentSidePanelModificationsEntity();
    QList<QGraphicsItem*> selItems = scene->selectedItems();
    bool successfullyStoredChanges = selItems.size() == 1 ?
        showSidePanelPropertiesFor(selItems[0]) :
        clearSidePanel();

    if (!handleSidePanelModificationsResult(successfullyStoredChanges, sidePanelEntityName))
        return;

    previouslySelectedItems = selItems;
}

void ErdWindow::reloadSchema()
{
    // TODO
}

void ErdWindow::commitPendingChanges()
{
    // TODO
}

void ErdWindow::rollbackPendingChanges()
{
    // TODO
}

void ErdWindow::handleCreatedChange(ErdChange* change)
{
    changeRegistry->addChange(change);
    scene->handleChange(change);
}

void ErdWindow::updateState()
{
    updateToolbarState(changeRegistry->getPendingChangesCount(),
                       changeRegistry->isUndoAvailable(),
                       changeRegistry->isRedoAvailable());
}

void ErdWindow::updateToolbarState(int effectiveChangeCount, bool undoAvailable, bool redoAvailable)
{
    bool hasPendingChanges = effectiveChangeCount > 0;
    actionMap[COMMIT]->setEnabled(hasPendingChanges);
    actionMap[ROLLBACK]->setEnabled(hasPendingChanges);
    actionMap[UNDO]->setEnabled(undoAvailable);
    actionMap[REDO]->setEnabled(redoAvailable);
    changeCountLabel->setText(QString::asprintf(CHANGE_COUNT_DIGITS, effectiveChangeCount));
}

void ErdWindow::abortSidePanel()
{
    ErdPropertiesPanel* propertiesPanel = dynamic_cast<ErdPropertiesPanel*>(currentSideWidget);
    if (propertiesPanel)
        propertiesPanel->abortErdChange();

    clearSidePanel();
}

void ErdWindow::showChangeRegistry()
{
    ErdChangeRegistryDialog dialog(memDb, changeRegistry, this);
    dialog.exec();
}

void ErdWindow::undo()
{
    ErdChange* change = changeRegistry->peekUndo();
    if (scene->undoChange(change))
        changeRegistry->undo();
}

void ErdWindow::redo()
{
    ErdChange* change = changeRegistry->peekRedo();
    if (scene->redoChange(change))
        changeRegistry->redo();
}

void ErdWindow::applyArrowType(ErdArrowItem::Type arrowType)
{
    CFG_ERD.Erd.ArrowType.set(arrowType);
    scene->setArrowType(arrowType);
}

bool ErdWindow::isUncommitted() const
{
    return false;
}

QString ErdWindow::getQuitUncommittedConfirmMessage() const
{
    return "";
}

void ErdWindow::setMdiWindow(MdiWindow* value)
{
    MdiChild::setMdiWindow(value);
    connect(value, &QMdiSubWindow::windowStateChanged, this, &ErdWindow::checkIfActivated);
}

bool ErdWindow::shouldReuseForArgs(int argCount, ...)
{
    if (argCount != 1)
        return false;

    va_list args;
    va_start(args, argCount);
    Db* argDb = va_arg(args, Db*);
    va_end(args);

    return argDb == db;
}

void ErdWindow::updateArrowTypeButtons()
{
    switch (scene->getArrowType())
    {
        case ErdArrowItem::STRAIGHT:
            actionMap[LINE_STRAIGHT]->setChecked(true);
            break;
        case ErdArrowItem::CURVY:
            actionMap[LINE_CURVY]->setChecked(true);
            break;
        case ErdArrowItem::SQUARE:
            actionMap[LINE_SQUARE]->setChecked(true);
            break;
    }
}

QToolBar* ErdWindow::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolBar;
}

QVariant ErdWindow::saveSession()
{
    QHash<QString,QVariant> sessionValue;
    if (db)
        sessionValue["db"] = db->getName();

    QHash<QString, QVariant> erdConfig;
    erdConfig.insert(scene->getConfig());
    erdConfig.insert(ui->view->getConfig());
    erdConfig[CFG_KEY_SPLITTER] = ui->splitter->saveState();

    if (db)
    {
        CFG->set(ERD_CFG_GROUP, db->getPath(), erdConfig);
        CFG->set(ERD_CFG_GROUP, db->getName(), erdConfig);
    }

    return sessionValue;
}

bool ErdWindow::storeEntityModifications(QWidget* sidePanelWidget)
{
    ErdPropertiesPanel* propertiesPanel = dynamic_cast<ErdPropertiesPanel*>(sidePanelWidget);
    if (propertiesPanel)
    {
        if (!propertiesPanel->commitErdChange())
            return false;
    }
    return true;
}

QString ErdWindow::getCurrentSidePanelModificationsEntity() const
{
    if (!currentSideWidget)
        return QString();

    ErdTableWindow* tableWin = qobject_cast<ErdTableWindow*>(currentSideWidget);
    if (tableWin)
        return tableWin->getTable();

    ErdConnectionPanel* connectionPanel = qobject_cast<ErdConnectionPanel*>(currentSideWidget);
    if (connectionPanel)
        return connectionPanel->getStartEntityTable();

    return QString();

}

void ErdWindow::parseAndRestore()
{
    if (!db)
        return;

    if (!initMemDb())
        return;

    QSet<QString> tableNames = scene->parseSchema();
    QVariant erdConfig = CFG->get(ERD_CFG_GROUP, db->getPath());
    if (!tryToApplyConfig(erdConfig, tableNames))
    {
        erdConfig = CFG->get(ERD_CFG_GROUP, db->getName());
        if (!tryToApplyConfig(erdConfig, tableNames))
            scene->arrangeEntitiesFdp(true);
    }
}

bool ErdWindow::clearSidePanel()
{
    if (!currentSideWidget)
        return true;

    if (!storeEntityModifications(currentSideWidget))
        return false;

    ui->sidePanel->layout()->removeWidget(currentSideWidget);
    currentSideWidget->deleteLater();
    currentSideWidget = nullptr;
    noSideWidgetContents->setVisible(true);
    ui->sidePanel->layout()->addWidget(noSideWidgetContents);
    return true;
}

bool ErdWindow::setSidePanelWidget(QWidget* widget)
{
    if (currentSideWidget && !storeEntityModifications(currentSideWidget))
        return false;

    if (currentSideWidget) // if still in place after storing (or not) entity updates & refreshing schema
    {
        ui->sidePanel->layout()->removeWidget(currentSideWidget);
        currentSideWidget->deleteLater();
    }
    else
    {
        ui->sidePanel->layout()->removeWidget(noSideWidgetContents);
        noSideWidgetContents->setVisible(false);
    }

    currentSideWidget = widget;
    ui->sidePanel->layout()->addWidget(widget);
    return true;
}

bool ErdWindow::showSidePanelPropertiesFor(QGraphicsItem* item)
{
    ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
    ErdArrowItem* arrow = dynamic_cast<ErdArrowItem*>(item);
    if (entity)
    {
        ErdTableWindow* tableMdiChild = new ErdTableWindow(memDb, entity);
        connect(tableMdiChild, &ErdTableWindow::changeCreated, this, &ErdWindow::handleCreatedChange, Qt::QueuedConnection);
        connect(tableMdiChild, &ErdTableWindow::editedEntityShouldBeDeleted, scene, &ErdScene::removeEntityFromScene, Qt::QueuedConnection);
        return setSidePanelWidget(tableMdiChild);
    }
    else if (arrow)
    {
        ErdConnection* connection = scene->getConnectionForArrow(arrow);
        if (!connection)
        {
            qCritical() << "No ErdConnection for ErdArrowItem!";
            return false;
        }

        if (!connection->isFinalized())
        {
            qDebug() << "Skipping selection action on connection, as it's not finalized connection.";
            return false;
        }

        if (connection->isCompoundConnection())
        {
            ignoreSelectionChangeEvents = true;
            for (ErdConnection*& assocConn : connection->getAssociatedConnections())
                assocConn->select(false);

            ignoreSelectionChangeEvents = false;
        }

        ErdConnectionPanel* panel = new ErdConnectionPanel(memDb, connection);
        connect(panel, &ErdConnectionPanel::changeCreated, this, &ErdWindow::handleCreatedChange, Qt::QueuedConnection);
        return setSidePanelWidget(panel);
    }
    return true;
}

bool ErdWindow::initMemDb()
{
    if (memDb)
    {
        memDb->closeQuiet();
        delete memDb;
    }

    memDb = DBLIST->createInMemDb();
    memDb->setName(db->getName()); // same name as original, so that all functions/collations/extensions are loaded when open
    if (!memDb->openQuiet())
    {
        qCritical() << "Failed to open in-memory database required for ERD editor! Db error:" << memDb->getErrorText();
        return false;
    }
    scene->setDb(memDb);

    // Now copy schema to memdb
    // Order must be: tables, views, triggers, other. It's because they can be created "on" tables" and "on views".
    SqlQueryPtr tableResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'table' AND name NOT LIKE 'sqlite_%'");
    SqlQueryPtr viewResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'view' AND name NOT LIKE 'sqlite_%'");
    SqlQueryPtr triggerResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'trigger' AND name NOT LIKE 'sqlite_%'");
    SqlQueryPtr indexResults = db->exec("SELECT sql FROM sqlite_schema WHERE type = 'index' AND name NOT LIKE 'sqlite_%'");

    QStringList ddls;
    for (const SqlQueryPtr& results : {tableResults, viewResults, triggerResults, indexResults})
    {
        for (SqlResultsRowPtr& row : results->getAll())
            ddls << row->value("sql").toString();
    }

    for (QString& ddl : ddls)
        memDb->exec(ddl);

    return true;
}

void ErdWindow::refreshSidePanel()
{
    if (!currentSideWidget)
        return;

    ErdTableWindow* tableWindow = qobject_cast<ErdTableWindow*>(currentSideWidget);
    if (tableWindow)
    {
        tableWindow->refreshStructure();
        return;
    }

    // Connection Panel will be destroyed upon entity refresh, because its connection item will be recreated.
    // No need to refresh that panel.
}

bool ErdWindow::storeCurrentSidePanelModifications()
{
    QString sidePanelEntityName = getCurrentSidePanelModificationsEntity();
    bool successfullyStoredChanges = clearSidePanel();
    return handleSidePanelModificationsResult(successfullyStoredChanges, sidePanelEntityName);
}

bool ErdWindow::handleSidePanelModificationsResult(bool successfullyStored, const QString& sidePanelEntityName)
{
    if (!sidePanelEntityName.isNull() && !successfullyStored)
    {
        QMessageBox::critical(this,
              tr("Table modification failed"),
              tr("There was a problem applying changes to the table '%1'. Check the messages panel for details. "
                 "Please either roll back the changes in the side panel or fix them. "
                 "You cannot continue editing the diagram until this issue is resolved.")
                              .arg(sidePanelEntityName)
            );

        ignoreSelectionChangeEvents = true;
        scene->clearSelection();
        for (QGraphicsItem*& item : previouslySelectedItems)
            item->setSelected(true);

        ignoreSelectionChangeEvents = false;
        return false;
    }
    return true;
}

bool ErdWindow::restoreSession(const QVariant& sessionValue)
{
    QHash<QString, QVariant> value = sessionValue.toHash();
    if (value.size() == 0)
        return false;

    if (!value.contains("db"))
        return false;

    QString dbName = value["db"].toString();
    Db* sessionDb = DBLIST->getByName(dbName);
    if (!sessionDb || !sessionDb->isOpen())
        return false;

    db = sessionDb;
    parseAndRestore();

    return true;
}

bool ErdWindow::tryToApplyConfig(const QVariant& value, const QSet<QString>& tableNames)
{
    if (!value.isValid() || value.isNull() || !value.canConvert<QHash<QString, QVariant>>())
        return false;

    QHash<QString, QVariant> erdConfig = value.toHash();

    // Is it applicable config for this db?
    StrHash<QVariant> cfgEntities = erdConfig[ErdScene::CFG_KEY_ENTITIES].toHash();
    int matched = 0;
    for (const QString& configTable : cfgEntities.lowerKeys())
    {
        if (tableNames.contains(configTable))
            matched++;
    }

    if (matched < 2 || ((qreal)matched / erdConfig.size()) < 0.25)
        return false;

    // Config is applicable.
    ui->splitter->restoreState(erdConfig[CFG_KEY_SPLITTER].toByteArray());
    scene->applyConfig(erdConfig);
    ui->view->applyConfig(erdConfig);
    updateArrowTypeButtons();

    return true;
}

void ErdWindow::focusItem(QGraphicsItem* item)
{
    scene->clearSelection();
    scene->setFocusItem(item, Qt::MouseFocusReason);
    item->setSelected(true);
}

Icon* ErdWindow::getIconNameForMdiWindow()
{
    return windowIcon;
}

QString ErdWindow::getTitleForMdiWindow()
{
    if (db)
        return tr("ERD editor (%1)").arg(db->getName());

    return tr("ERD editor");
}
