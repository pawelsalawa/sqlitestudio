#ifndef INDEXEXPRCOLUMNDIALOG_H
#define INDEXEXPRCOLUMNDIALOG_H

#include <QDialog>

class SqliteExpr;
class Db;

namespace Ui {
    class IndexExprColumnDialog;
}

class IndexExprColumnDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit IndexExprColumnDialog(Db* db, QWidget *parent = 0);
        IndexExprColumnDialog(Db* db, SqliteExpr* col, QWidget *parent = 0);
        ~IndexExprColumnDialog();

        SqliteExpr* getColumn() const;
        void setTableColumns(const QStringList& value);
        void setExistingExprColumnKeys(const QStringList& value);

    private:
        void readColumn(SqliteExpr* col);
        void setOkEnabled(bool enabled);
        SqliteExpr* parseExpr();
        bool checkRestrictions(QString& errorMsg);

        Ui::IndexExprColumnDialog *ui;
        SqliteExpr* theColumn = nullptr;
        QString lastValidatedText;
        Db* db = nullptr;
        QStringList tableColumns;
        QStringList existingExprColumnKeys;

    public slots:
        void accept();
        int exec();

    private slots:
        void validate();
};

#endif // INDEXEXPRCOLUMNDIALOG_H
