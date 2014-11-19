#ifndef MESSAGELISTDIALOG_H
#define MESSAGELISTDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class MessageListDialog;
}

class GUI_API_EXPORT MessageListDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit MessageListDialog(QWidget *parent = 0);
        explicit MessageListDialog(const QString& message, QWidget *parent = 0);
        ~MessageListDialog();

        void addMessage(const QString& text, const QBrush& background = QBrush());
        void addMessage(const QIcon& icon, const QString& text, const QBrush& background = QBrush());
        void addInfo(const QString& text);
        void addWarning(const QString& text);
        void addError(const QString& text);

    protected:
        void changeEvent(QEvent *e);
        void showEvent(QShowEvent*);
        void resizeEvent(QResizeEvent*);

    private:
        QBrush getGradient(qreal r, qreal g, qreal b, qreal a) const;

        Ui::MessageListDialog *ui = nullptr;
};

#endif // MESSAGELISTDIALOG_H
