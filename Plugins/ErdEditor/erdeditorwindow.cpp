#include "erdeditorwindow.h"
#include "icon.h"
#include "iconmanager.h"
#include "ui_erdeditorwindow.h"
#include "erdscene.h"
#include "common/unused.h"
#include "services/dbmanager.h"
#include "mdiwindow.h"
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
    windowIcon->load();

    scene = new ErdScene(this);
    ui->graphView->setScene(scene);

    initActions();

    scene->parseSchema(db);
}

void ErdEditorWindow::checkIfActivated(Qt::WindowStates oldState, Qt::WindowStates newState)
{
    if (!oldState.testFlag(Qt::WindowActive) && newState.testFlag(Qt::WindowActive))
        ui->graphView->setFocus();
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

void ErdEditorWindow::createActions()
{
    // TODO
    createAction(NEW_TABLE, ICONS.TABLE_ADD, tr("Create a &table"), scene, SLOT(newTable()), ui->toolBar);
    createAction(ARRANGE, ICONS.TABLE_ADD, tr("Arrange entities"), scene, SLOT(arrangeEntities()), ui->toolBar);
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
