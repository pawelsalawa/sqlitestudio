#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>
#include <QColor>

class ColorButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit ColorButton(QWidget *parent = 0);

        QColor getColor() const;
        void setColor(const QColor& value);

    protected:
        void resizeEvent(QResizeEvent* e);

    private:
        QColor color;

    private slots:
        void pickColor();

    signals:
        void colorChanged(const QColor& color);
};

#endif // COLORBUTTON_H
