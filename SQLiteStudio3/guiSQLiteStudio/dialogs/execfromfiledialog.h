#ifndef EXECFROMFILEDIALOG_H
#define EXECFROMFILEDIALOG_H

#include <QDialog>

namespace Ui {
    class ExecFromFileDialog;
}

class ExecFromFileDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit ExecFromFileDialog(QWidget *parent = nullptr);
        ~ExecFromFileDialog();

        bool ignoreErrors() const;
        QString filePath() const;
        QString codec() const;

    private:
        void init();

        Ui::ExecFromFileDialog *ui;

    private slots:
        void browseForInputFile();
        void updateState();
};

#endif // EXECFROMFILEDIALOG_H
