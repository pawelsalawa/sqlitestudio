#ifndef DELETEONFOCUSOUTFILTER_H
#define DELETEONFOCUSOUTFILTER_H

#include <QObject>
#include <QSet>

class DeleteOnFocusOutFilter : public QObject
{
    Q_OBJECT

    public:
        using QObject::QObject;

        void ignoredReason(Qt::FocusReason reason);

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        QSet<Qt::FocusReason> ignoredFocusReasons;
};

#endif // DELETEONFOCUSOUTFILTER_H
