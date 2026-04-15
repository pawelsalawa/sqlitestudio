#include "dialogsizehandler.h"
#include "services/config.h"
#include <QEvent>
#include <QTimer>
#include <QWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

DialogSizeHandler::DialogSizeHandler(const QString &key, QObject *parent, Mode mode) :
    QObject(parent), configKey(key)
{
    saveTimer = new QTimer(this);
    saveTimer->setInterval(500);
    saveTimer->setSingleShot(true);
    connect(saveTimer, SIGNAL(timeout()), this, SLOT(doSave()));

    QRect geom = CFG->get(CONFIG_GROUP, configKey).toRect();
    if (geom.isValid() && qApp->primaryScreen()->geometry().contains(geom))
    {
        QWidget* w = qobject_cast<QWidget*>(parent);
        switch (mode)
        {
            case BOTH:
                w->setGeometry(geom);
                break;
            case HORIZONTAL:
                w->setGeometry(geom.x(), geom.y(), geom.width(), w->height());
                break;
            case VERTICAL:
                w->setGeometry(geom.x(), geom.y(), w->width(), geom.height());
                break;
        }
    }
}

DialogSizeHandler::~DialogSizeHandler()
{
}

void DialogSizeHandler::applyFor(QObject *parent, Mode mode)
{
    QString key = parent->objectName();
    if (key.isEmpty())
        key = parent->metaObject()->className();

    applyFor(key, parent, mode);
}

void DialogSizeHandler::applyFor(const QString &key, QObject *parent, Mode mode)
{
    DialogSizeHandler* handler = new DialogSizeHandler(key, parent, mode);
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
