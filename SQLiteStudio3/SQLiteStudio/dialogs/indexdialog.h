#ifndef INDEXDIALOG_H
#define INDEXDIALOG_H

#include "db/db.h"
#include "parser/ast/sqlitecreateindex.h"
#include <QDialog>
#include <QStringListModel>

namespace Ui {
    class IndexDialog;
}

class QGridLayout;
class QSignalMapper;
class QCheckBox;
class QComboBox;

class IndexDialog : public QDialog
{
        Q_OBJECT

    public:
        IndexDialog(Db* db, QWidget *parent = 0);
        IndexDialog(Db* db, const QString& index, QWidget *parent = 0);
        ~IndexDialog();

        void setTable(const QString& value);

    protected:
        void changeEvent(QEvent *e);

    private:
        void init();
        void readIndex();
        void readCollations();
        void buildColumn(const QString& name, int row);
        void applyColumnValues();
        void applyIndex();
        SqliteIndexedColumn* addIndexedColumn(const QString& name);
        void rebuildCreateIndex();

        bool existingIndex = false;
        Db* db = nullptr;
        QString table;
        QString index;
        SqliteCreateIndexPtr createIndex;
        SqliteCreateIndexPtr originalCreateIndex;
        QStringList tableColumns;
        QGridLayout* columnsLayout;
        QSignalMapper* columnStateSignalMapping;
        QStringListModel collations;
        QList<QCheckBox*> columnCheckBoxes;
        QList<QComboBox*> sortComboBoxes;
        QList<QComboBox*> collateComboBoxes;
        int totalColumns = 0;
        Ui::IndexDialog *ui;

    private slots:
        void updateValidation();
        void buildColumns();
        void updateTable(const QString& value);
        void updateColumnState(int row);
        void updatePartialConditionState();
        void updateDdl();
        void tabChanged(int tab);

    public slots:
        void accept();
};

#endif // INDEXDIALOG_H
