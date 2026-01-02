#ifndef DELETEONFOCUSOUTFILTER_H
#define DELETEONFOCUSOUTFILTER_H

#include <QObject>

class DeleteOnFocusOutFilter : public QObject
{
    Q_OBJECT

    public:
        using QObject::QObject;

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // DELETEONFOCUSOUTFILTER_H
