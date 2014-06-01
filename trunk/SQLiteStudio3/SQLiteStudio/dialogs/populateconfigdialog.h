#ifndef POPULATECONFIGDIALOG_H
#define POPULATECONFIGDIALOG_H

#include <QDialog>

class PopulateEngine;

namespace Ui {
    class PopulateConfigDialog;
}

class PopulateConfigDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit PopulateConfigDialog(PopulateEngine* engine, const QString& column, const QString& pluginName, QWidget *parent = 0);
        ~PopulateConfigDialog();

        int exec();

    private:
        Ui::PopulateConfigDialog *ui;
        PopulateEngine* engine = nullptr;
};

#endif // POPULATECONFIGDIALOG_H
