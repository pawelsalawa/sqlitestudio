#include "userinputfilter.h"
#include "common/unused.h"
#include <QTimer>
#include <QLineEdit>

UserInputFilter::UserInputFilter(QLineEdit* lineEdit, QObject* filterHandler, const char* handlerSlot) :
    QObject(lineEdit),
    lineEdit(lineEdit)
{
    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(200);
    connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(filterModified(QString)));
    connect(timer, SIGNAL(timeout()), this, SLOT(applyFilter()));
    connect(this, SIGNAL(applyFilter(QString)), filterHandler, handlerSlot);
}

void UserInputFilter::setDelay(int msecs)
{
    timer->setInterval(msecs);
}

void UserInputFilter::filterModified(const QString& newValue)
{
    UNUSED(newValue);
    timer->start();
}

void UserInputFilter::applyFilter()
{
    timer->stop();
    emit applyFilter(lineEdit->text());
}
