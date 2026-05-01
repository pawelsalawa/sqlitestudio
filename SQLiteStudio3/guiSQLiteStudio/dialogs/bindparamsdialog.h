#ifndef BINDPARAMSDIALOG_H
#define BINDPARAMSDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QHash>


class QLineEdit;
namespace Ui {
    class BindParamsDialog;
}

struct BindParam;
class MultiEditor;

class GUI_API_EXPORT BindParamsDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit BindParamsDialog(QWidget *parent = nullptr);
        ~BindParamsDialog();

        void setBindParams(const QVector<BindParam*>& params);

    private:
        void init();
        void initMultiEditors(const QVector<QPair<QString, QVariant>>& savedParams);
        void initSimpleEditors(const QVector<QPair<QString, QVariant>>& savedParams);
        bool needsMultiEditor(const QVector<QPair<QString, QVariant>>& savedParams) const;
        MultiEditor* initMultiEditor(BindParam* param, const QVariant& cachedValue);
        QVector<QPair<QString, QVariant>> getSavedParams() const;
        QVector<QPair<QString, QVariant> > collectCurrentValues() const;
        void clearCurrentEditors();

        static const int multiMargins = 2;
        static const int multiSpacing = 2;
        static const int simpleMargins = 9;
        static const int simpleSpacing = 6;
        static const int minimumFieldHeight = 120;

        Ui::BindParamsDialog *ui;
        QVector<BindParam*> bindParams;
        QHash<BindParam*, MultiEditor*> multiEditors;
        QHash<BindParam*, QLineEdit*> simpleEditors;
        QWidget* contents = nullptr;

    private slots:
        void toggleMode(bool advanced);

    public slots:
        void accept();
};

#endif // BINDPARAMSDIALOG_H
