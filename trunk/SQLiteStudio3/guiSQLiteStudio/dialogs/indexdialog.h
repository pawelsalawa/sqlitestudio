#ifndef INDEXDIALOG_H
#define INDEXDIALOG_H

#include "db/db.h"
#include "guiSQLiteStudio_global.h"
#include "parser/ast/sqlitecreateindex.h"
#include "common/strhash.h"
#include <QDialog>
#include <QStringListModel>

namespace Ui {
    class IndexDialog;
}

class QGridLayout;
class QSignalMapper;
class QCheckBox;
class QComboBox;
class QTableWidget;

class GUI_API_EXPORT IndexDialog : public QDialog
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
        class Column
        {
            public:
                Column(const QString& name, QTableWidget* table);

                void assignToNewRow(int row);
                void prepareForNewRow();
                QCheckBox* getCheck();
                void setCheck(QCheckBox* cb);
                QWidget* getCheckParent();
                void setCheckParent(QWidget* w);
                QComboBox* getSort();
                void setSort(QComboBox* cb);
                QComboBox* getCollation();
                void setCollation(QComboBox* cb);
                bool hasCollation() const;

                QString getName() const;

            private:
                QWidget* defineContainer(QWidget* w);

                QWidget* column1Contrainer = nullptr;
                QWidget* column2Contrainer = nullptr;
                QWidget* column3Contrainer = nullptr;
                QWidget* checkParent = nullptr;
                QCheckBox* check = nullptr;
                QComboBox* sort = nullptr;
                QComboBox* collation = nullptr;
                QTableWidget* table = nullptr;
                QString name;
        };

        void init();
        void readIndex();
        void readCollations();
        void buildColumn(const QString& name, int row);
        void applyColumnValues();
        void applyIndex();
        SqliteIndexedColumn* addIndexedColumn(const QString& name);
        void rebuildCreateIndex();
        void queryDuplicates();
        void clearColumns();
        void rebuildColumnsByNewOrder();

        bool existingIndex = false;
        Db* db = nullptr;
        QString table;
        QString index;
        SqliteCreateIndexPtr createIndex;
        SqliteCreateIndexPtr originalCreateIndex;
        QStringList tableColumns;
        QSignalMapper* columnStateSignalMapping = nullptr;
        QStringListModel collations;
        StrHash<Column*> columns;
        QList<Column*> columnsByRow;
        int totalColumns = 0;
        Ui::IndexDialog *ui = nullptr;

    private slots:
        void updateValidation();
        void buildColumns();
        void updateTable(const QString& value);
        void updateColumnState(const QString& rowName);
        void updatePartialConditionState();
        void updateDdl();
        void tabChanged(int tab);
        void moveColumnUp();
        void moveColumnDown();
        void updateUpDownButtons(const QModelIndex& idx = QModelIndex());

    public slots:
        void accept();
};

#endif // INDEXDIALOG_H
