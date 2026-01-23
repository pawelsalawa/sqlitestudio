#ifndef COLORPICKERPOPUP_H
#define COLORPICKERPOPUP_H

#include <QWidget>
#include <QHash>

class QSignalMapper;
class QVBoxLayout;
class QToolButton;
class ColorPickerPopup : public QWidget
{
        Q_OBJECT
    public:
        explicit ColorPickerPopup(QWidget *parent = nullptr);
        ~ColorPickerPopup();

        void addCustomColor(const QColor& c);
        QVector<QColor> getCustomColors();
        void setCustomColors(const QVector<QColor>& colors);

        static void staticInit();

    private:
        void init();
        QWidget* createColorGrid(const QVector<QColor>& colors, int columns);
        void refreshCustomColors();
        QToolButton* createColorButton(const QColor& color);
        void clearHighlight();
        void highlightButton(QToolButton* btn);

        static QVector<QColor> baseColors;
        static const int cellSize = 20;

        QWidget* customColorsWidget = nullptr;
        QVBoxLayout* mainLayout = nullptr;
        QSignalMapper* colorButtonMapper = nullptr;
        QVector<QColor> customColors;
        QHash<QRgb, QToolButton*> colorButtons;
        QToolButton* currentHighlighted = nullptr;

    public slots:
        void markColor(const QColor& color);
        void clearColorMark();

    private slots:
        void pickCustomColor();
        void handleColorRgbaClick(int mappedColor);
        void handleColorClick(const QColor& color);

    signals:
        void colorPicked(const QColor& color);
        void resetRequested();
};

#endif // COLORPICKERPOPUP_H
