#ifndef ERDCONNECTIONPANEL_H
#define ERDCONNECTIONPANEL_H

#include <QWidget>
#include "common/extactioncontainer.h"

namespace Ui {
    class ErdConnectionPanel;
}

class ErdChange;
class Db;
class ErdEntity;
class ErdConnection;

class GUI_API_EXPORT ErdConnectionPanel : public QWidget, public ExtActionContainer
{
        Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR_MAIN
        };

        ErdConnectionPanel(Db* db, ErdConnection* connection, QWidget *parent = nullptr);
        ~ErdConnectionPanel();

        QString getStartEntityTable() const;

    protected:
        void createActions();
        void setupDefShortcuts();
        QToolBar *getToolBar(int toolbar) const;

    private:
        void init();
        void initColumnLevelFk();
        void initTableLevelFk();

        Ui::ErdConnectionPanel *ui;
        Db* db = nullptr;
        ErdConnection* connection = nullptr;

    signals:
        void changeCreated(ErdChange* change);
};

#endif // ERDCONNECTIONPANEL_H
