#ifndef WIDGETCOVER_H
#define WIDGETCOVER_H

#include "guiSQLiteStudio_global.h"
#include <QWidget>
#include <QEasingCurve>
#include <QVariant>

class QVariantAnimation;
class QGridLayout;
class QPushButton;
class QProgressBar;

class GUI_API_EXPORT WidgetCover : public QWidget
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
        void displayProgress(int maxValue, const QString& format = QString());
        void noDisplayProgress();
        void initWithProgressBarOnly(const QString& format);
        void initWithInterruptContainer(const QString& interruptButtonText = QString());

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
        QPushButton* cancelButton = nullptr;
        QProgressBar* busyBar = nullptr;
        bool undetermined = false;

    signals:
        void cancelClicked();

    private slots:
        void animationUpdate(const QVariant& value);
        void animationFinished();

    public slots:
        void show();
        void hide();
        void setProgress(int value);
};

#endif // WIDGETCOVER_H
