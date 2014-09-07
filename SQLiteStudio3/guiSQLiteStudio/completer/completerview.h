#ifndef COMPLETERLIST_H
#define COMPLETERLIST_H

#include "guiSQLiteStudio_global.h"
#include <QListWidget>

class CompleterView : public QListView
{
        Q_OBJECT

    public:
        explicit CompleterView(QWidget *parent = 0);

        void selectFirstVisible();
        bool hasVisibleItem() const;
        int countVisibleItem() const;

    protected:
        void focusOutEvent(QFocusEvent* e);
        void keyPressEvent(QKeyEvent* e);

    signals:
        void focusOut();
        void textTyped(const QString& text);
        void backspace();
        void left();
        void right();
};

#endif // COMPLETERLIST_H
