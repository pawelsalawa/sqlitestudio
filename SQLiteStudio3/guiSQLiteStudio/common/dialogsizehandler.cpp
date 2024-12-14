#include "dialogsizehandler.h"
#include "services/config.h"
#include <QEvent>
#include <QTimer>
#include <QWidget>

DialogSizeHandler::DialogSizeHandler(QObject *parent) :
    DialogSizeHandler(parent->objectName(), parent)
{
}

DialogSizeHandler::DialogSizeHandler(const QString &key, QObject *parent) :
    QObject(parent), configKey(key)
{
    saveTimer = new QTimer(this);
    saveTimer->setInterval(500);
    saveTimer->setSingleShot(true);
    connect(saveTimer, SIGNAL(timeout()), this, SLOT(doSave()));

    QRect geom = CFG->get(CONFIG_GROUP, configKey).toRect();
    if (geom.isValid())
    {
        QWidget* w = qobject_cast<QWidget*>(parent);
        w->setGeometry(geom);
    }
}

DialogSizeHandler::~DialogSizeHandler()
{
}

void DialogSizeHandler::applyFor(QObject *parent)
{
    applyFor(parent->objectName(), parent);
}

void DialogSizeHandler::applyFor(const QString &key, QObject *parent)
{
    DialogSizeHandler* handler = new DialogSizeHandler(key, parent);
    parent->installEventFilter(handler);
}

bool DialogSizeHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize || event->type() == QEvent::Move)
    {
        QWidget* w = qobject_cast<QWidget*>(obj);
        recentGeometry = w->geometry();
        saveTimer->start();
    }
    return false;
}

void DialogSizeHandler::doSave()
{
    CFG->set(CONFIG_GROUP, configKey, recentGeometry);
}
