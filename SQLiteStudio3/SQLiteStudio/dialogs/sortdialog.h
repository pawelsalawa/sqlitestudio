#ifndef SORTDIALOG_H
#define SORTDIALOG_H

#include "db/queryexecutor.h"
#include "common/extactioncontainer.h"
#include <QDialog>

namespace Ui {
    class SortDialog;
}

class QTreeWidgetItem;

class SortDialog : public QDialog, public ExtActionContainer
{
        Q_OBJECT

    public:
        enum Action
        {
            MOVE_UP,
            MOVE_DOWN
        };

        enum ToolBar
        {
        };

        explicit SortDialog(QWidget *parent = 0);
        ~SortDialog();

        void setColumns(const QStringList& columns);
        QueryExecutor::SortList getSortOrder() const;
        void setSortOrder(const QueryExecutor::SortList& sortOrder);
        QToolBar* getToolBar(int toolbar) const;

    protected:
        void createActions();
        void setupDefShortcuts();

    private:
        void updateState(QTreeWidgetItem* item);
        void updateState();
        void fixItemFlags(QTreeWidgetItem* item);
        void rebuildComboForItem(QTreeWidgetItem* item);
        void updateSortLabel();

        Ui::SortDialog *ui;
        QStringList originalColumns;

    private slots:
        void itemChanged(QTreeWidgetItem* item, int column);
        void reset();
        void updateButtons();
        void moveCurrentUp();
        void moveCurrentDown();
};

#endif // SORTDIALOG_H
