#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include "guiSQLiteStudio_global.h"
#include <QPushButton>
#include <QColor>

class GUI_API_EXPORT ColorButton : public QPushButton
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
