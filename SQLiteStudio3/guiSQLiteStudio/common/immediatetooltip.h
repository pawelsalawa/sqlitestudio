#ifndef IMMEDIATETOOLTIP_H
#define IMMEDIATETOOLTIP_H

#include <QObject>

class ImmediateTooltip : public QObject
{
    Q_OBJECT
    public:
        explicit ImmediateTooltip(QWidget *parent = nullptr);

        const QString& getToolTip() const;
        void setToolTip(const QString& newToolTip);

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    private:
        QString toolTip;
};

#endif // IMMEDIATETOOLTIP_H
