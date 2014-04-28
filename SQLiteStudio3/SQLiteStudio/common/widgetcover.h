#ifndef WIDGETCOVER_H
#define WIDGETCOVER_H

#include <QWidget>
#include <QEasingCurve>
#include <QVariant>

class QVariantAnimation;
class QGridLayout;

class WidgetCover : public QWidget
{
    Q_OBJECT

    public:
        explicit WidgetCover(QWidget *parent);
        explicit WidgetCover(const QEasingCurve& easingCurve, QWidget *parent);
        virtual ~WidgetCover();

        QEasingCurve getEasingCurve() const;
        void setEasingCurve(const QEasingCurve& value);

        int getDuration() const;
        void setDuration(int value);

        int getTransparency() const;
        void setTransparency(int value);

        QGridLayout* getContainerLayout();
        bool eventFilter(QObject* obj, QEvent* e);

    private:
        enum class Action
        {
            SHOWING,
            HIDING,
            NONE
        };

        void init();
        void interruptAction();
        void resetBackground();
        void widgetResized();

        Action actionInProgres = Action::NONE;
        QVariantAnimation* animation = nullptr;
        QEasingCurve easingCurve = QEasingCurve::OutCubic;
        int duration = 150;
        int transparency = 128;
        QWidget* container = nullptr;
        QGridLayout* containerLayout = nullptr;

    private slots:
        void animationUpdate(const QVariant& value);
        void animationFinished();

    public slots:
        void show();
        void hide();
};

#endif // WIDGETCOVER_H
