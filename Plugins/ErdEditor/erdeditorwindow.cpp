#include "erdeditorwindow.h"
#include "icon.h"
#include "iconmanager.h"
#include "ui_erdeditorwindow.h"
#include "erdscene.h"
#include "common/unused.h"
#include "services/dbmanager.h"
#include "mdiwindow.h"
#include "style.h"
#include <QDebug>
#include <QMdiSubWindow>

ErdEditorWindow::ErdEditorWindow() :
    ui(new Ui::ErdEditorWindow)
{
    init();
}

ErdEditorWindow::ErdEditorWindow(QWidget *parent, Db* db) :
    MdiChild(parent),
    ui(new Ui::ErdEditorWindow), db(db)
{
    init();
}

ErdEditorWindow::ErdEditorWindow(const ErdEditorWindow& other) :
    MdiChild(other.parentWidget()),
    ui(new Ui::ErdEditorWindow), db(other.db)
{
    init();
}

ErdEditorWindow::~ErdEditorWindow()
{
    delete ui;
    delete windowIcon;
    delete fdpIcon;
    delete neatoIcon;
}

void ErdEditorWindow::staticInit()
{
    if (!QMetaType::isRegistered(QMetaType::type("ErdEditorWindow")))
        qRegisterMetaType<ErdEditorWindow>("ErdEditorWindow");
}

void ErdEditorWindow::init()
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

    connect(STYLE, &Style::paletteChanged, this, &ErdEditorWindow::uiPaletteChanged);

    scene->parseSchema(db);
}

void ErdEditorWindow::checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState)
{
    if (!oldState.testFlag(Qt::WindowActive) && newState.testFlag(Qt::WindowActive))
        ui->graphView->setFocus();
}

void ErdEditorWindow::uiPaletteChanged()
{
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    scene->update();
}

void ErdEditorWindow::addFk()
{

}

void ErdEditorWindow::useStraightLine()
{

}

void ErdEditorWindow::useCurvyLine()
{

}

bool ErdEditorWindow::isUncommitted() const
{
    return false; // TODO
}

QString ErdEditorWindow::getQuitUncommittedConfirmMessage() const
{
    return ""; // TODO
}

void ErdEditorWindow::setMdiWindow(MdiWindow* value)
{
    MdiChild::setMdiWindow(value);
    connect(value, &QMdiSubWindow::windowStateChanged, this, &ErdEditorWindow::checkIfActivated);
}

bool ErdEditorWindow::shouldReuseForArgs(int argCount, ...)
{
    if (argCount != 1)
        return false;

    va_list args;
    va_start(args, argCount);
    Db* argDb = va_arg(args, Db*);
    va_end(args);

    return argDb == db;
}

void ErdEditorWindow::createActions()
{
    createAction(NEW_TABLE, ICONS.TABLE_ADD, tr("Create a &table"), scene, SLOT(newTable()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(ADD_CONNECTION, *connectionIcon, tr("Add foreign key"), scene, SLOT(addFk()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(LINE_STRAIGHT, *lineStraightIcon, tr("Use straight line"), scene, SLOT(useStraightLine()), ui->toolBar);
    createAction(LINE_CURVY, *lineCurvyIcon, tr("Use curvy line"), scene, SLOT(useCurvyLine()), ui->toolBar);
    ui->toolBar->addSeparator();
    createAction(ARRANGE_FDP, *fdpIcon, tr("Arrange entities using Force-Directed Placement approach"), scene, SLOT(arrangeEntitiesFdp()), ui->toolBar);
    createAction(ARRANGE_NEATO, *neatoIcon, tr("Arrange entities using Spring Model approach"), scene, SLOT(arrangeEntitiesNeato()), ui->toolBar);
}

void ErdEditorWindow::setupDefShortcuts()
{
    // TODO
}

QToolBar* ErdEditorWindow::getToolBar(int toolbar) const
{
    UNUSED(toolbar);
    return ui->toolBar;
}

QVariant ErdEditorWindow::saveSession()
{
    QHash<QString,QVariant> sessionValue;
    if (db)
        sessionValue["db"] = db->getName();

    return sessionValue;
}

bool ErdEditorWindow::restoreSession(const QVariant& sessionValue)
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

Icon* ErdEditorWindow::getIconNameForMdiWindow()
{
    return windowIcon;
}

QString ErdEditorWindow::getTitleForMdiWindow()
{
    if (db)
        return tr("ERD editor (%1)").arg(db->getName());

    return tr("ERD editor");
}
