#ifndef BUGREPORTHISTORYWINDOW_H
#define BUGREPORTHISTORYWINDOW_H

#include "mdichild.h"
#include <QWidget>

namespace Ui {
    class BugReportHistoryWindow;
}

CFG_KEY_LIST(BugReportHistoryWindow, QObject::tr("Reports history window"),
     CFG_KEY_ENTRY(DELETE_SELECTED, Qt::Key_Delete, QObject::tr("Delete selected entry"))
)

class GUI_API_EXPORT BugReportHistoryWindow : public MdiChild
{
        Q_OBJECT
        Q_ENUMS(Action)

    public:
        enum Action
        {
            DELETE_SELECTED,
            CLEAR_HISTORY
        };

        enum ToolBar
        {
            TOOLBAR
        };

        explicit BugReportHistoryWindow(QWidget *parent = 0);
        ~BugReportHistoryWindow();

        bool restoreSessionNextTime();
        bool isUncommitted() const;
        QString getQuitUncommittedConfirmMessage() const;

    protected:
        QVariant saveSession();
        bool restoreSession(const QVariant &sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();
        QToolBar* getToolBar(int toolbar) const;

    private:
        enum UserRole
        {
            ENTRY_ID = Qt::UserRole + 1
        };

        void init();

        Ui::BugReportHistoryWindow *ui = nullptr;

    private slots:
        void updateState();
        void reload();
        void clearHistory();
        void deleteSelected();
};

#endif // BUGREPORTHISTORYWINDOW_H
