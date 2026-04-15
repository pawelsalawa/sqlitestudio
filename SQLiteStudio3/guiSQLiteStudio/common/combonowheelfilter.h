#ifndef COMBONOWHEELFILTER_H
#define COMBONOWHEELFILTER_H

#include <QObject>

class ComboNoWheelFilter : public QObject
{
        Q_OBJECT

    public:
        explicit ComboNoWheelFilter(QObject* parent = nullptr);

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

};

#endif // COMBONOWHEELFILTER_H
