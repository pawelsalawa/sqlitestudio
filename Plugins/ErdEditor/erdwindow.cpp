#include "erdwindow.h"
#include "icon.h"
#include "iconmanager.h"
#include "ui_erdwindow.h"
#include "erdscene.h"
#include "common/unused.h"
#include "services/dbmanager.h"
#include "mdiwindow.h"
#include "style.h"
#include "erdeditorplugin.h"
#include "services/config.h"
#include "erdentity.h"
#include "erdtablewindow.h"
#include "erdchange.h"
#include "erdchangeregistry.h"
#include "erdchangeentity.h"
#include "db/sqlquery.h"
#include "db/sqlresultsrow.h"
#include "tablemodifier.h"
#include <QDebug>
#include <QMdiSubWindow>
#include <QActionGroup>
#include <QShortcut>
#include <QGraphicsOpacityEffect>

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
}

void ErdWindow::init()
{
    ui->setupUi(this);

    for (auto icon : {windowIcon, fdpIcon, neatoIcon, lineStraightIcon, lineCurvyIcon, lineSquareIcon})
        icon->load();

#ifdef Q_OS_MACOS
    ui->bottomLabel->setText(ui->bottomLabel->text().replace("Ctrl+F", "Cmd+F"));
#endif

    ui->splitter->setSizes({400, 200});
    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 1);

    noSideWidgetContents = ui->noContentWidget;
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(noSideWidgetContents);
    opacityEffect->setOpacity(0.5);
    noSideWidgetContents->setGraphicsEffect(opacityEffect);

    ErdArrowItem::Type arrowType = (ErdArrowItem::Type)CFG_ERD.Erd.ArrowType.get();

    escHotkey = new QShortcut(QKeySequence::Cancel, this, SLOT(cancelCurrentAction()), SLOT(cancelCurrentAction()), Qt::WidgetWithChildrenShortcut);

    scene = new ErdScene(arrowType, this);
    ui->graphView->setScene(scene);

    initActions();

    connect(STYLE, &Style::paletteChanged, this, &ErdWindow::uiPaletteChanged);
    connect(scene, &QGraphicsScene::selectionChanged, this, &ErdWindow::itemSelectionChanged);
    connect(actionMap[ADD_CONNECTION], SIGNAL(toggled(bool)), ui->graphView, SLOT(setDraftingConnectionMode(bool)));
    connect(ui->graphView, &ErdView::draftConnectionRemoved, [this]()
    {
        actionMap[ADD_CONNECTION]->setChecked(false);
    });

    changeRegistry = new ErdChangeRegistry(this);

    parseAndRestore();
}

void ErdWindow::checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState)
{
    if (!oldState.testFlag(Qt::WindowActive) && newState.testFlag(Qt::WindowActive))
        ui->graphView->setFocus();
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

void ErdWindow::useCurvyLine()
{
    applyArrowType(ErdArrowItem::CURVY);
}

void ErdWindow::useSquareLine()
{
    applyArrowType(ErdArrowItem::SQUARE);
}

void ErdWindow::cancelCurrentAction()
{
    if (actionMap[ADD_CONNECTION]->isChecked())
    {
        ui->graphView->abortDraftConnection();
        return;
    }
}

void ErdWindow::newTable()
{
    SqliteCreateTable* tableModel = new SqliteCreateTable();
    tableModel->table = tr("table name", "ERD editor");

    ErdEntity* entity = new ErdEntity(tableModel);
    entity->setExistingTable(false);
    scene->placeNewEntity(entity);

    selectItem(entity);
}

void ErdWindow::itemSelectionChanged()
{
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    if (selectedItems.size() != 1)
        clearSidePanel();
    else
        showSidePanelPropertiesFor(selectedItems[0]);
}

void ErdWindow::commitPendingChanges()
{

}

void ErdWindow::rollbackPendingChanges()
{

}

void ErdWindow::handleCreatedChange(ErdChange* change)
{
    changeRegistry->addChange(change);

    ErdChangeEntity* entityChange = dynamic_cast<ErdChangeEntity*>(change);
    if (!entityChange)
        return;

    ErdEntity* entity = entityChange->getEntity();

    TableModifier* tableModifier = entityChange->getTableModifier();
    QStringList modifiedTables = {entity->getTableName()};
    modifiedTables += tableModifier->getModifiedTables();

    scene->refreshSchema(memDb, modifiedTables);
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

void ErdWindow::createActions()
{
    actionMap[CANCEL_CURRENT] = new QAction(tr("Cancels ongoing action"), this);
    connect(actionMap[CANCEL_CURRENT], &QAction::triggered, this, &ErdWindow::cancelCurrentAction);

    QActionGroup* lineGroup = new QActionGroup(ui->toolBar);
    lineGroup->setExclusive(true);
    actionMap[ADD_CONNECTION] = new QAction(ICONS.CONSTRAINT_FOREIGN_KEY, tr("Add foreign key"), this);
    actionMap[LINE_STRAIGHT] = new QAction(*lineStraightIcon, tr("Use straight line"), lineGroup);
    actionMap[LINE_CURVY] = new QAction(*lineCurvyIcon, tr("Use curvy line"), lineGroup);
    actionMap[LINE_SQUARE] = new QAction(*lineSquareIcon, tr("Use square line"), lineGroup);

    actionMap[ADD_CONNECTION]->setCheckable(true);
    actionMap[LINE_STRAIGHT]->setCheckable(true);
    actionMap[LINE_CURVY]->setCheckable(true);
    actionMap[LINE_SQUARE]->setCheckable(true);
    updateArrowTypeButtons();

    connect(actionMap[LINE_STRAIGHT], &QAction::triggered, this, &ErdWindow::useStraightLine);
    connect(actionMap[LINE_CURVY], &QAction::triggered, this, &ErdWindow::useCurvyLine);
    connect(actionMap[LINE_SQUARE], &QAction::triggered, this, &ErdWindow::useSquareLine);

    createAction(COMMIT, ICONS.COMMIT, tr("Commit all pending changes", "ERD editor"), this, SLOT(commitPendingChanges()), ui->toolBar);
    createAction(ROLLBACK, ICONS.ROLLBACK, tr("Rollback all pending changes", "ERD editor"), this, SLOT(rollbackPendingChanges()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(NEW_TABLE, ICONS.TABLE_ADD, tr("Create a &table"), this, SLOT(newTable()), ui->toolBar);
    ui->toolBar->addSeparator();
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
    erdConfig.insert(ui->graphView->getConfig());

    CFG->set(ERD_CFG_GROUP, db->getPath(), erdConfig);
    CFG->set(ERD_CFG_GROUP, db->getName(), erdConfig);

    return sessionValue;
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

    for (const QString& ddl : ddls)
        memDb->exec(ddl);

    return true;
}

void ErdWindow::parseAndRestore()
{
    if (!db)
        return;

    if (!initMemDb())
        return;

    QSet<QString> tableNames = scene->parseSchema(memDb);
    QVariant erdConfig = CFG->get(ERD_CFG_GROUP, db->getPath());
    if (!tryToApplyConfig(erdConfig, tableNames))
    {
        erdConfig = CFG->get(ERD_CFG_GROUP, db->getName());
        if (!tryToApplyConfig(erdConfig, tableNames))
            scene->arrangeEntitiesFdp(true);
    }
}

void ErdWindow::clearSidePanel()
{
    if (currentSideWidget)
    {
        ui->sidePanel->layout()->removeWidget(currentSideWidget);
        currentSideWidget->deleteLater();
        currentSideWidget = nullptr;
        ui->sidePanel->layout()->addWidget(noSideWidgetContents);
    }
}

void ErdWindow::setSidePanelWidget(QWidget* widget)
{
    if (currentSideWidget)
    {
        ui->sidePanel->layout()->removeWidget(currentSideWidget);
        currentSideWidget->deleteLater();
    }
    else
        ui->sidePanel->layout()->removeWidget(noSideWidgetContents);

    currentSideWidget = widget;
    ui->sidePanel->layout()->addWidget(widget);
}

void ErdWindow::showSidePanelPropertiesFor(QGraphicsItem* item)
{
    ErdEntity* entity = dynamic_cast<ErdEntity*>(item);
    if (entity)
    {
        ErdTableWindow* tableMdiChild = new ErdTableWindow(memDb, entity);
        connect(tableMdiChild, &ErdTableWindow::changeCreated, this, &ErdWindow::handleCreatedChange);
        setSidePanelWidget(tableMdiChild);
    }
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
    StrHash<QVariant> cfgEntities = erdConfig[ErdScene::CFG_KEY_ENTITIES].toHash();
    int matched = 0;
    for (QString configTable : cfgEntities.lowerKeys())
    {
        if (tableNames.contains(configTable))
            matched++;
    }

    if (matched < 2 || ((qreal)matched / erdConfig.size()) < 0.25)
        return false;

    scene->applyConfig(erdConfig);
    ui->graphView->applyConfig(erdConfig);
    updateArrowTypeButtons();

    return true;
}

void ErdWindow::selectItem(QGraphicsItem* item)
{
    scene->clearSelection();
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
