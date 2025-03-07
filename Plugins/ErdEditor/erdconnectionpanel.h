#ifndef ERDCONNECTIONPANEL_H
#define ERDCONNECTIONPANEL_H

#include <QWidget>

namespace Ui {
    class ErdConnectionPanel;
}

class ErdChange;
class Db;
class ErdEntity;
class ErdConnection;

class ErdConnectionPanel : public QWidget
{
        Q_OBJECT

    public:
        ErdConnectionPanel(Db* db, ErdConnection* connection, QWidget *parent = nullptr);
        ~ErdConnectionPanel();

        QString getStartEntityTable() const;

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
