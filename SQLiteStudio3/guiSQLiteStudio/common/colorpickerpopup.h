#ifndef COLORPICKERPOPUP_H
#define COLORPICKERPOPUP_H

#include <QWidget>

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

        static void staticInit();

    private:
        QWidget* createColorGrid(const QVector<QColor>& colors, int columns);
        void refreshCustomColors();
        QToolButton* createColorButton(const QColor& color);

        QWidget* customColorsWidget = nullptr;
        QVBoxLayout* mainLayout = nullptr;
        QSignalMapper* colorButtonMapper = nullptr;
        QVector<QColor> customColors;

        static QVector<QColor> baseColors;
        static const int cellSize = 20;

    private slots:
        void pickCustomColor();
        void handleColorRgbaClick(int mappedColor);
        void handleColorClick(const QColor& color);

    signals:
        void colorPicked(const QColor& color);
        void resetRequested();
};

#endif // COLORPICKERPOPUP_H
