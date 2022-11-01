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
            DoubleClick,
            Wheel
        };

        MouseShortcut(MouseShortcut::ClickType type,
                      Qt::MouseButtons buttons,
                      Qt::KeyboardModifiers modifiers,
                      QObject *parent = 0);

        static MouseShortcut* forWheel(Qt::KeyboardModifiers modifiers,
                      QObject *parent = 0);

        static MouseShortcut* forWheel(Qt::KeyboardModifiers modifiers,
                                       QObject *parent, const char *slot);
        static MouseShortcut* forWheel(Qt::KeyboardModifiers modifiers,
                                       QObject* receiver, const char* slot, QObject* parent);
    protected:
        bool eventFilter(QObject *object, QEvent *event);

    private:
        bool attributesMatch(QEvent* event);

        MouseShortcut::ClickType type;
        Qt::MouseButtons buttons;
        Qt::KeyboardModifiers modifiers;

    signals:
        void activated();
        void wheelActivated(int delta);
};

#endif // MOUSESHORTCUT_H
