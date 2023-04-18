#include "erdeditorwindow.h"
#include "icon.h"
#include "iconmanager.h"
#include "parser/ast/sqlitecreatetable.h"
#include "ui_erdeditorwindow.h"
#include "erdscene.h"
#include "erdentity.h"
#include "erdarrowitem.h"
#include "common/unused.h"
#include <QDebug>

ErdEditorWindow::ErdEditorWindow(QWidget *parent) :
    MdiChild(parent),
    ui(new Ui::ErdEditorWindow)
{
    init();
}

ErdEditorWindow::ErdEditorWindow(const ErdEditorWindow& other) :
    MdiChild(other.parentWidget()),
    ui(new Ui::ErdEditorWindow)
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

    initActions();

    scene = new ErdScene(this);
//    scene->setSceneRect(QRectF(QPointF(0, 0), QSizeF(ui->graphView->viewport()->size())));
    ui->graphView->setScene(scene);

//    qApp->processEvents(QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeSocketNotifiers);

//    // Create the first entity item
//    ErdEntity* entityItem1 = new ErdEntity();
//    entityItem1->setPos(-200, -200);
//    entityItem1->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
//    scene->addItem(entityItem1);

//    // Create the second entity item
//    ErdEntity* entityItem2 = new ErdEntity();
//    entityItem2->setPos(200, 200);
//    entityItem2->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
//    scene->addItem(entityItem2);

//    QPen pen(Qt::white);

//    QGraphicsLineItem* line1 = new QGraphicsLineItem();
//    line1->setPen(pen);
//    line1->setLine(0, -10, 0, 10);
//    scene->addItem(line1);

//    QGraphicsLineItem* line2 = new QGraphicsLineItem();
//    line2->setPen(pen);
//    line2->setLine(-10, 0, 10, 0);
//    scene->addItem(line2);

//    qApp->processEvents(QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeSocketNotifiers);

//    // Line
//    QGraphicsPathItem* pathItem = new QGraphicsPathItem();
//    pathItem->setPen(pen);
//    QPainterPath path;
//    qDebug() << entityItem1->geometry();
//    path.moveTo(entityItem1->geometry().right(), -180);
//    path.cubicTo(150, -160, 0, 0, 200, 220);
//    pathItem->setPath(path);
    //    scene->addItem(pathItem);
}

void ErdEditorWindow::newTable()
{
    // Create the first entity item
    SqliteCreateTable* tableModel = new SqliteCreateTable();
    tableModel->table = "test table " + QString::number(lastCreatedX);

    SqliteCreateTable::Column* col1 = new SqliteCreateTable::Column("test col", nullptr, {});
    col1->setParent(tableModel);
    tableModel->columns << col1;

    SqliteCreateTable::Column* col2 = new SqliteCreateTable::Column("another col", nullptr, {});
    col2->setParent(tableModel);
    tableModel->columns << col2;

    ErdEntity* entityItem = new ErdEntity(tableModel);
    entityItem->setPos(lastCreatedX, -200);
    entityItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    scene->addItem(entityItem);

    lastCreatedX += 150;

    entities << entityItem;

//    if (entities.size() > 1)
//    {
//        qApp->processEvents(QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeSocketNotifiers);

//        QPointF p1 = entities[0]->pos();
//        QPointF p2 = entities[1]->pos();
//        ErdArrowItem* arrow = new ErdArrowItem(QLineF(p1, p2));
//        scene->addItem(arrow);
//    }
}

bool ErdEditorWindow::isUncommitted() const
{
    return false; // TODO
}

QString ErdEditorWindow::getQuitUncommittedConfirmMessage() const
{
    return ""; // TODO
}

void ErdEditorWindow::createActions()
{
    // TODO
    createAction(NEW_TABLE, ICONS.TABLE_ADD, tr("Create a &table"), this, SLOT(newTable()), ui->toolBar);
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
    return QVariant(); // TODO
}

bool ErdEditorWindow::restoreSession(const QVariant& sessionValue)
{
    UNUSED(sessionValue);
    return true;
}

Icon* ErdEditorWindow::getIconNameForMdiWindow()
{
    return windowIcon;
}

QString ErdEditorWindow::getTitleForMdiWindow()
{
    return tr("ERD editor");
}
