#ifndef WIDGETSTATEINDICATOR_H
#define WIDGETSTATEINDICATOR_H

#include <QObject>

class QLabel;
class QGraphicsDropShadowEffect;
class QGraphicsColorizeEffect;
class QSequentialAnimationGroup;

class WidgetStateIndicator : public QObject
{
        Q_OBJECT
    public:
        enum class Mode
        {
            INFO,
            ERROR,
            WARNING,
            HINT
        };

        ~WidgetStateIndicator();

        void setMode(Mode mode);
        void show(const QString& msg = QString());
        void hide();
        void setVisible(bool visible, const QString& msg = QString());

        static WidgetStateIndicator* getInstance(QWidget* widget);

    protected:
        bool eventFilter(QObject *obj, QEvent *ev);

    private:
        explicit WidgetStateIndicator(QWidget *widget);

        void initLabel();
        void initEffects();
        void initGlowEffects();
        void initHighlightingEffects();
        void initAnimations();
        void finalInit();
        void setMessage(const QString& msg);
        void clearMessage();
        void detectWindowParent();
        QWidget* findParentWindow(QWidget* w);
        bool shouldHide();
        bool shouldShow();

        QWidget* labelParent = nullptr;
        QLabel* label = nullptr;
        Mode mode = Mode::ERROR;
        QWidget* widget = nullptr;
        QString message;
        QGraphicsColorizeEffect* highlightingEffect = nullptr;
        QGraphicsDropShadowEffect* glowEffect = nullptr;
        QSequentialAnimationGroup* animation = nullptr;
        bool widgetVisible = false;
        bool visibilityRequested = false;
        QWidget* windowParent = nullptr;

        static QHash<QWidget*,WidgetStateIndicator*> instances;

    private slots:
        void updateMode();
        void updatePosition();
        void updateVisibility();

};

#define INDICATOR(w) WidgetStateIndicator::getInstance(w)

#endif // WIDGETSTATEINDICATOR_H
