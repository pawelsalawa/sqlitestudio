#ifndef FONTEDIT_H
#define FONTEDIT_H

#include <QWidget>

namespace Ui {
    class FontEdit;
}

class FontEdit : public QWidget
{
        Q_OBJECT

        Q_PROPERTY(QFont font READ getFont WRITE setFont NOTIFY fontChanged)

    public:
        explicit FontEdit(QWidget *parent = 0);
        ~FontEdit();

        QFont getFont() const;

    public slots:
        void setFont(QFont arg);

    protected:
        void changeEvent(QEvent *e);

    private:
        void init();
        void updateFont();

        Ui::FontEdit *ui;
        QFont font;

    private slots:
        void browse();

    signals:
        void fontChanged(const QFont& font);
};

#endif // FONTEDIT_H
