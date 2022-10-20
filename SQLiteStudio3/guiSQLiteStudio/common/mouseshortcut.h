#ifndef MOUSESHORTCUT_H
#define MOUSESHORTCUT_H

#include <QObject>

class MouseShortcut : public QObject
{
    Q_OBJECT

    public:
        enum ClickType
        {
            SingleClick,
            DoubleClick
        };

        MouseShortcut(MouseShortcut::ClickType type,
                      Qt::MouseButtons buttons,
                      Qt::KeyboardModifiers modifiers,
                      QObject *parent = 0);
    protected:
        bool eventFilter(QObject *object, QEvent *event);

    private:
        bool attributesMatch(QEvent* event);

        MouseShortcut::ClickType type;
        Qt::MouseButtons buttons;
        Qt::KeyboardModifiers modifiers;

    signals:
        void activated();
};

#endif // MOUSESHORTCUT_H
