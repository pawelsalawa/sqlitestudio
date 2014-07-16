#ifndef DBCONVERTERDIALOG_H
#define DBCONVERTERDIALOG_H

#include <QDialog>

class DbListModel;
class Db;
class DbVersionConverter;
class WidgetCover;

namespace Ui {
    class DbConverterDialog;
}

class DbConverterDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DbConverterDialog(QWidget *parent = 0);
        ~DbConverterDialog();

        void setDb(Db* db);

    private:
        void init();
        void srcDbChanged();
        bool validate();

        static bool confirmConversion(const QList<QPair<QString, QString> >& diffs);
        static bool confirmConversionErrors(const QSet<QString>& errors);

        Ui::DbConverterDialog *ui;
        DbListModel* dbListModel = nullptr;
        Db* srcDb = nullptr;
        DbVersionConverter* converter = nullptr;
        bool dontUpdateState = false;
        WidgetCover* widgetCover = nullptr;

    public slots:
        void accept();

    private slots:
        void srcDbChanged(int index);
        void updateState();
        void processingFailed(const QString& errorMessage);
        void processingSuccessful();
        void processingAborted();
};

#endif // DBCONVERTERDIALOG_H
