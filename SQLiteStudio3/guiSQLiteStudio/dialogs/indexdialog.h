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
                /**
                 * @brief Constructor for a real column in the table
                 */
                Column(const QString& name, QTableWidget* table);

                /**
                 * @brief Constructor for an expression column
                 */
                Column(SqliteExpr* expr, QTableWidget* table);
                ~Column();

                void assignToNewRow(int row);
                void prepareForNewRow();
                QCheckBox* getCheck() const;
                void setCheck(QCheckBox* cb);
                QWidget* getCheckParent() const;
                void setCheckParent(QWidget* w);
                QComboBox* getSort() const;
                void setSort(QComboBox* cb);
                QComboBox* getCollation() const;
                void setCollation(QComboBox* cb);

                /**
                 * @brief Gets name of a real column in the table
                 * @return Name of table's column, or null string if it's expression column.
                 */
                QString getName() const;

                /**
                 * @brief Gets expression of an expression column.
                 * @return Either the expression, or null if this is a real table column.
                 *
                 * If an expression assigned to the column is a collate expression,
                 * this method will extract inner expression and return that.
                 *
                 * This is done this way, because after parsing SqliteOrderBy,
                 * the whole expression (including outer COLLATE expression) is passed
                 * to the column, whereas COLLATE is used for dropdowns, not for actual expression.
                 */
                SqliteExpr* getExpr() const;

                void setExpr(SqliteExpr* expr);

                /**
                 * @brief Tells whether this is an expression column or real table column.
                 * @return true if it's expression column, or false if it's real table.
                 */
                bool isExpr() const;
                QString getKey() const;

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
                SqliteExpr* expr = nullptr;
        };

        static QString buildKey(SqliteExpr* expr);

        void init();
        void readIndex();
        void readCollations();
        void buildColumn(const QString& name, int row);
        Column* buildColumn(SqliteOrderBy* orderBy, int row);
        Column* buildColumn(SqliteExpr* expr, int row);
        void buildColumn(Column* column, int row);
        void applyColumnValues();
        void applyIndex();
        SqliteOrderBy* addIndexedColumn(const QString& name);
        SqliteOrderBy* addIndexedColumn(SqliteExpr* expr);
        void addCollation(SqliteOrderBy* col, const QString& name);
        void rebuildCreateIndex();
        void queryDuplicates();
        void clearColumns();
        void rebuildColumnsByNewOrder();
        QString getKey(SqliteOrderBy* col) const;
        QStringList getExistingColumnExprs(const QString& exceptThis = QString()) const;
        QStringList getTableColumns() const;
        void preReject();
        QString getOriginalDdl() const;

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
        bool preRejected = false;
        Ui::IndexDialog *ui = nullptr;

    private slots:
        void updateValidation();
        void buildColumns();
        void updateTable(const QString& value);
        void updateColumnState(const QString& columnKey);
        void updatePartialConditionState();
        void updateDdl();
        void tabChanged(int tab);
        void moveColumnUp();
        void moveColumnDown();
        void updateToolBarButtons(const QModelIndex& idx = QModelIndex());
        void addExprColumn();
        void editExprColumn(int row = -1);
        void delExprColumn();

    public slots:
        void accept();
        int exec();
};

#endif // INDEXDIALOG_H
