#ifndef MATERIALSWITCH_H
#define MATERIALSWITCH_H

#include <QCheckBox>
#include <QPropertyAnimation>

class MaterialSwitch : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)

    public:
        enum LabelPosition
        {
            LabelLeft,
            LabelRight
        };

        explicit MaterialSwitch(QWidget *parent = nullptr);
        MaterialSwitch(const QString& title, QWidget *parent = nullptr);

        QSize sizeHint() const override;

        qreal offset() const;
        void setOffset(qreal value);

        void setLabelPosition(LabelPosition pos);
        LabelPosition labelPosition() const;

        void setLabelAlignment(Qt::Alignment align);
        Qt::Alignment labelAlignment() const;

    protected:
        void paintEvent(QPaintEvent *event) override;
        void resizeEvent(QResizeEvent *e) override;
        bool hitButton(const QPoint &pos) const override;

    private slots:
        void startTransition(bool checked);

    private:
        QRect switchRect() const;
        QRect labelRect() const;

        qreal offsetValue;
        QPropertyAnimation *animation;
        LabelPosition labelPos = LabelRight;
        Qt::Alignment labelAlign = Qt::AlignLeft;
        int m_spacing = 6;
    };

#endif // MATERIALSWITCH_H
