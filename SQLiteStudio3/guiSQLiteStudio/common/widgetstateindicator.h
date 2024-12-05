#ifndef WIDGETSTATEINDICATOR_H
#define WIDGETSTATEINDICATOR_H

#include "guiSQLiteStudio_global.h"
#include <QObject>

class QLabel;
class QGraphicsDropShadowEffect;
class QGraphicsColorizeEffect;

class GUI_API_EXPORT WidgetStateIndicator : public QObject
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

        enum class PositionMode
        {
            DEFAULT,
            GROUP_BOX,
            LABEL,
            CHECK_BOX,
        };

        ~WidgetStateIndicator();

        void setMode(Mode mode);
        void show(const QString& msg = QString());
        void hide();
        void setVisible(bool visible, const QString& msg = QString());
        bool isVisible() const;
        void release();
        void info(const QString& msg);
        void warn(const QString& msg);
        void error(const QString& msg);
        void hint(const QString& msg);

        static bool exists(QWidget* widget);
        static WidgetStateIndicator* getInstance(QWidget* widget);

        PositionMode getPositionMode() const;
        void setPositionMode(const PositionMode& value);

    protected:
        bool eventFilter(QObject *obj, QEvent *ev);

    private:
        explicit WidgetStateIndicator(QWidget *widget);

        void initLabel();
        void initEffects();
        void initGlowEffects();
        void initHighlightingEffects();
        void initPositionMode();
        void finalInit();
        void setMessage(const QString& msg);
        void clearMessage();
        void detectWindowParent();
        QWidget* findParentWindow(QWidget* w);
        bool shouldHide();
        bool shouldShow();
        bool eventFilterFromWidget(QEvent *ev);
        bool eventFilterFromParentWidget(QEvent *ev);
        bool eventFilterFromIndicatorLabel(QEvent *ev);

        QWidget* labelParent = nullptr;
        QLabel* label = nullptr;
        Mode mode = Mode::ERROR;
        QWidget* widget = nullptr;
        QString message;
        QString processedMessage;
        QGraphicsColorizeEffect* highlightingEffect = nullptr;
        QGraphicsDropShadowEffect* glowEffect = nullptr;
        bool widgetVisible = false;
        bool visibilityRequested = false;
        QWidget* windowParent = nullptr;
        PositionMode positionMode = PositionMode::DEFAULT;

        static QHash<QWidget*,WidgetStateIndicator*> instances;

    private slots:
        void updateMode();
        void updatePosition();
        void updatePositionDefault();
        void updatePositionGroupBox();
        void updatePositionLabel();
        void updatePositionCheckBox();
        void updateVisibility();

};

#define INDICATOR(w) WidgetStateIndicator::getInstance(w)
#define EXISTS_INDICATOR(w) WidgetStateIndicator::exists(w)

#endif // WIDGETSTATEINDICATOR_H
