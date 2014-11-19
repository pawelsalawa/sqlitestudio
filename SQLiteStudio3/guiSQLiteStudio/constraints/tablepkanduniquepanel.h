#ifndef PRIMARYKEYANDUNIQUEPANEL_H
#define PRIMARYKEYANDUNIQUEPANEL_H

#include "parser/ast/sqlitecreatetable.h"
#include "constraintpanel.h"
#include "guiSQLiteStudio_global.h"
#include <QStringListModel>

namespace Ui {
    class TablePrimaryKeyAndUniquePanel;
}

class QGridLayout;
class QSignalMapper;

class GUI_API_EXPORT TablePrimaryKeyAndUniquePanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit TablePrimaryKeyAndUniquePanel(QWidget *parent = 0);
        ~TablePrimaryKeyAndUniquePanel();

        bool validate();
        void storeConfiguration();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        virtual void readConstraint();

        Ui::TablePrimaryKeyAndUniquePanel *ui = nullptr;
        QGridLayout* columnsLayout = nullptr;

        /**
         * @brief totalColumns
         * Used to count columns created by buildColumns().
         * It's a workaround for QGridLayout::rowCount(), which doesn't return
         * number of visible rows, but instead a number of allocated rows,
         * which isn't useful in any way.
         * This variable lets us to avoid asking for widgets from rows
         * that does not exist.
         */
        int totalColumns = 0;

    private:
        void init();
        void readCollations();
        void buildColumns();
        void buildColumn(SqliteCreateTable::Column* column, int row);
        int getColumnIndex(const QString& colName);

        QStringListModel collations;
        QSignalMapper* columnSignalMapping = nullptr;

    private slots:
        void updateColumnState(int colIdx);

    protected slots:
        virtual void updateState();
};

#endif // PRIMARYKEYANDUNIQUEPANEL_H
