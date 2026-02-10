#ifndef COMBONOWHEELFILTER_H
#define COMBONOWHEELFILTER_H

#include <QObject>

class ComboNoWheelFilter : public QObject
{
        Q_OBJECT

    public:
        using QObject::QObject;

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

};

#endif // COMBONOWHEELFILTER_H
