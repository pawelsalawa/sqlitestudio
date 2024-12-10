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
#include <QDebug>
#include <QMdiSubWindow>
#include <QActionGroup>
#include <QShortcut>

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
    delete ui;
    delete windowIcon;
    delete fdpIcon;
    delete neatoIcon;
}

void ErdWindow::staticInit()
{
    qRegisterMetaType<ErdWindow>("ErdWindow");
}

void ErdWindow::init()
{
    ui->setupUi(this);

    arrowType = (ErdArrowItem::Type)CFG_ERD.Erd.ArrowType.get();

    escHotkey = new QShortcut(QKeySequence::Cancel, this, SLOT(cancelCurrentAction()), SLOT(cancelCurrentAction()), Qt::WidgetWithChildrenShortcut);

    windowIcon = new Icon("ERD_EDITOR", "erdeditor");
    fdpIcon = new Icon("ERDLAYOUT_FDP", "erdlayout_fdp");
    neatoIcon = new Icon("ERDLAYOUT_NEATO", "erdlayout_neato");
    lineStraightIcon = new Icon("ERDEDITOR_LINE_STRAIGHT", "erdeditor_line_straight");
    lineCurvyIcon = new Icon("ERDEDITOR_LINE_CURVY", "erdeditor_line_curvy");
    lineSquareIcon = new Icon("ERDEDITOR_LINE_SQUARE", "erdeditor_line_square");

    for (auto icon : {windowIcon, fdpIcon, neatoIcon, lineStraightIcon, lineCurvyIcon, lineSquareIcon})
        icon->load();

    scene = new ErdScene(arrowType, this);
    ui->graphView->setScene(scene);

    initActions();

    connect(STYLE, &Style::paletteChanged, this, &ErdWindow::uiPaletteChanged);
    connect(actionMap[ADD_CONNECTION], SIGNAL(toggled(bool)), ui->graphView, SLOT(setDraftingConnectionMode(bool)));
    connect(ui->graphView, &ErdView::draftConnectionRemoved, [this]()
    {
        actionMap[ADD_CONNECTION]->setChecked(false);
    });

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
    arrowType = ErdArrowItem::STRAIGHT;
    applyArrowType();
}

void ErdWindow::useCurvyLine()
{
    arrowType = ErdArrowItem::CURVY;
    applyArrowType();
}

void ErdWindow::useSquareLine()
{
    arrowType = ErdArrowItem::SQUARE;
    applyArrowType();
}

void ErdWindow::cancelCurrentAction()
{
    if (actionMap[ADD_CONNECTION]->isChecked())
    {
        ui->graphView->abortDraftConnection();
        return;
    }
}

void ErdWindow::applyArrowType()
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
    switch (arrowType)
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

    connect(actionMap[LINE_STRAIGHT], &QAction::triggered, this, &ErdWindow::useStraightLine);
    connect(actionMap[LINE_CURVY], &QAction::triggered, this, &ErdWindow::useCurvyLine);
    connect(actionMap[LINE_SQUARE], &QAction::triggered, this, &ErdWindow::useSquareLine);

    createAction(NEW_TABLE, ICONS.TABLE_ADD, tr("Create a &table"), scene, SLOT(newTable()), ui->toolBar);
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

    QHash<QString, QVariant> erdLayout = scene->getConfig();
    CFG->set(ERD_CFG_GROUP, db->getPath(), erdLayout);
    CFG->set(ERD_CFG_GROUP, db->getName(), erdLayout);

    return sessionValue;
}

void ErdWindow::parseAndRestore()
{
    if (!db)
        return;

    QSet<QString> tableNames = scene->parseSchema(db);
    QVariant erdConfig = CFG->get(ERD_CFG_GROUP, db->getPath());
    if (!tryToApplyConfig(erdConfig, tableNames))
    {
        erdConfig = CFG->get(ERD_CFG_GROUP, db->getName());
        if (!tryToApplyConfig(erdConfig, tableNames))
            scene->arrangeEntitiesFdp();
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
    if (!value.isValid() || value.isNull() || !value.canConvert<QHash<QString, QVariant>>()) /*value.canConvert(QMetaType(QMetaType::QVariantHash))*/
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

    return true;
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
