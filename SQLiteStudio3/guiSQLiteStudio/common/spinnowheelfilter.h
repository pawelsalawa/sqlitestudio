#ifndef SPINNOWHEELFILTER_H
#define SPINNOWHEELFILTER_H

#include <QObject>

class SpinNoWheelFilter : public QObject
{
        Q_OBJECT

    public:
        explicit SpinNoWheelFilter(QObject* parent = nullptr);

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // SPINNOWHEELFILTER_H
