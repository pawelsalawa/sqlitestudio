#ifndef DDLHISTORYWINDOW_H
#define DDLHISTORYWINDOW_H

#include "mdichild.h"

namespace Ui {
    class DdlHistoryWindow;
}

class QStringListModel;
class UserInputFilter;
class DdlHistoryModel;

class GUI_API_EXPORT DdlHistoryWindow : public MdiChild
{
        Q_OBJECT

    public:
        enum ToolBar
        {
        };

        explicit DdlHistoryWindow(QWidget *parent = 0);
        ~DdlHistoryWindow();

        bool restoreSessionNextTime();
        bool isUncommitted() const;
        QString getQuitUncommittedConfirmMessage() const;

    protected:
        void changeEvent(QEvent *e);
        QVariant saveSession();
        bool restoreSession(const QVariant& sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();
        QToolBar* getToolBar(int toolbar) const;

    private:
        void init();

        Ui::DdlHistoryWindow *ui = nullptr;
        QStringListModel* dbListModel = nullptr;
        DdlHistoryModel* dataModel = nullptr;
        UserInputFilter* filter = nullptr;

    private slots:
        void activated(const QModelIndex& current, const QModelIndex& previous);
        void applyFilter(const QString& filterValue);
        void refreshDbList();
        void clearHistory();
};

#endif // DDLHISTORYWINDOW_H
