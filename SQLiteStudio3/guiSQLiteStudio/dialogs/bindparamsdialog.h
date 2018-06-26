#ifndef BINDPARAMSDIALOG_H
#define BINDPARAMSDIALOG_H

#include <QDialog>

namespace Ui {
    class BindParamsDialog;
}

class BindParam;
class MultiEditor;

class BindParamsDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit BindParamsDialog(QWidget *parent = nullptr);
        ~BindParamsDialog();

        void setBindParams(const QVector<BindParam*>& params);

    private:
        void init();
        void initEditors();
        void initEditor(BindParam* param);

        static const int margins = 2;
        static const int spacing = 2;
        static const int minimumFieldHeight = 30;

        Ui::BindParamsDialog *ui;
        QVector<BindParam*> bindParams;
        QHash<BindParam*, MultiEditor*> editors;
        QWidget* contents = nullptr;

    public slots:
        void accept();
};

#endif // BINDPARAMSDIALOG_H
