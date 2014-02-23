#ifndef EXTLINEEDIT_H
#define EXTLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class ExtLineEdit : public QLineEdit
{
    Q_OBJECT

    public:
        explicit ExtLineEdit(QWidget *parent = 0);
        explicit ExtLineEdit(const QString& text, QWidget *parent = 0);

        bool getExpanding() const;
        void setExpanding(bool value);

        int getExpandingMinWidth() const;
        void setExpandingMinWidth(int value);

        int getExpandingMaxWidth() const;
        void setExpandingMaxWidth(int value);

        void setClearButtonEnabled(bool enable);

    private:
        void init();
        void updateMinSize();

        static const int expandingExtraSpace = 4; // QLineEdit has hardcoded horizontal margin of 2 for both sides

        bool expanding = false;
        int expandingMinWidth = 0;
        int expandingMaxWidth = -1;

    private slots:
        void handleTextChanged();

    signals:
        void valueErased();
};

#endif // EXTLINEEDIT_H
