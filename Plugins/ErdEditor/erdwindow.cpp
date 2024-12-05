#include "erdwindow.h"
#include "common/extaction.h"
#include "icon.h"
#include "iconmanager.h"
#include "ui_erdwindow.h"
#include "erdscene.h"
#include "common/unused.h"
#include "services/dbmanager.h"
#include "mdiwindow.h"
#include "style.h"
#include <QDebug>
#include <QMdiSubWindow>
#include <QActionGroup>

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

    windowIcon = new Icon("ERD_EDITOR", "erdeditor");
    fdpIcon = new Icon("ERDLAYOUT_FDP", "erdlayout_fdp");
    neatoIcon = new Icon("ERDLAYOUT_NEATO", "erdlayout_neato");
    connectionIcon = new Icon("ERDEDITOR_CONNECTION", "erdeditor_connection");
    lineStraightIcon = new Icon("ERDEDITOR_LINE_STRAIGHT", "erdeditor_line_straight");
    lineCurvyIcon = new Icon("ERDEDITOR_LINE_CURVY", "erdeditor_line_curvy");

    for (auto icon : {windowIcon, fdpIcon, neatoIcon, connectionIcon, lineStraightIcon, lineCurvyIcon})
        icon->load();

    scene = new ErdScene(this);
    ui->graphView->setScene(scene);

    initActions();

    connect(STYLE, &Style::paletteChanged, this, &ErdWindow::uiPaletteChanged);

    scene->parseSchema(db);
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

void ErdWindow::addFk()
{

}

void ErdWindow::useStraightLine()
{

}

void ErdWindow::useCurvyLine()
{

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
    actionMap[ADD_CONNECTION] = new ExtAction(*connectionIcon, tr("Add foreign key"), this);
    actionMap[LINE_STRAIGHT] = new ExtAction(*lineStraightIcon, tr("Use straight line"), this);
    actionMap[LINE_CURVY] = new ExtAction(*lineCurvyIcon, tr("Use curvy line"), this);
    actionMap[ADD_CONNECTION]->setCheckable(true);
    actionMap[LINE_STRAIGHT]->setCheckable(true);
    actionMap[LINE_CURVY]->setCheckable(true);

    QActionGroup* lineGroup = new QActionGroup(this);
    lineGroup->addAction(actionMap[LINE_STRAIGHT]);
    lineGroup->addAction(actionMap[LINE_CURVY]);

    createAction(NEW_TABLE, ICONS.TABLE_ADD, tr("Create a &table"), scene, SLOT(newTable()), ui->toolBar);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(actionMap[ADD_CONNECTION]);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(actionMap[LINE_STRAIGHT]);
    ui->toolBar->addAction(actionMap[LINE_CURVY]);
    ui->toolBar->addSeparator();
    createAction(ARRANGE_FDP, *fdpIcon, tr("Arrange entities using Force-Directed Placement approach"), scene, SLOT(arrangeEntitiesFdp()), ui->toolBar);
    createAction(ARRANGE_NEATO, *neatoIcon, tr("Arrange entities using Spring Model approach"), scene, SLOT(arrangeEntitiesNeato()), ui->toolBar);
}

void ErdWindow::setupDefShortcuts()
{
    // TODO
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

    return sessionValue;
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
    scene->parseSchema(db);
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
