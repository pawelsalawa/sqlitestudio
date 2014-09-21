#ifndef QUITCONFIRMDIALOG_H
#define QUITCONFIRMDIALOG_H

#include <QDialog>

namespace Ui {
    class QuitConfirmDialog;
}

class QuitConfirmDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit QuitConfirmDialog(QWidget *parent = 0);
        ~QuitConfirmDialog();

        void addMessage(const QString& msg);
        void setMessages(const QStringList& messages);
        int getMessageCount() const;

    private:
        void init();

        Ui::QuitConfirmDialog *ui;
};

#endif // QUITCONFIRMDIALOG_H
