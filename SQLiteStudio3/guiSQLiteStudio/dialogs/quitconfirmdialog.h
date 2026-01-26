#ifndef QUITCONFIRMDIALOG_H
#define QUITCONFIRMDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class QuitConfirmDialog;
}

class GUI_API_EXPORT QuitConfirmDialog : public QDialog
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

        Ui::QuitConfirmDialog *ui = nullptr;
};

#endif // QUITCONFIRMDIALOG_H
