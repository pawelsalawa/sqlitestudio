#ifndef DDLHISTORYWINDOW_H
#define DDLHISTORYWINDOW_H

#include "mdichild.h"

namespace Ui {
    class DdlHistoryWindow;
}

class QStringListModel;
class UserInputFilter;
class DdlHistoryModel;

class DdlHistoryWindow : public MdiChild
{
        Q_OBJECT

    public:
        explicit DdlHistoryWindow(QWidget *parent = 0);
        ~DdlHistoryWindow();

        bool restoreSessionNextTime();

    protected:
        void changeEvent(QEvent *e);
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        QString getIconNameForMdiWindow();
        QString getTitleForMdiWindow();

    private:
        void init();

        Ui::DdlHistoryWindow *ui;
        QStringListModel* dbListModel;
        DdlHistoryModel* dataModel;
        UserInputFilter* filter;

    private slots:
        void activated(const QModelIndex& current, const QModelIndex& previous);
        void applyFilter(const QString& filterValue);
        void refreshDbList();
};

#endif // DDLHISTORYWINDOW_H
