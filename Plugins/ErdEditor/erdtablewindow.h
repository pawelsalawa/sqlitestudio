#ifndef ERDTABLEWINDOW_H
#define ERDTABLEWINDOW_H

#include "windows/tablewindow.h"
#include <QObject>

class ErdEntity;
class ErdChange;

class ErdTableWindow : public TableWindow
{
        Q_OBJECT

    public:
        ErdTableWindow(Db* db, ErdEntity* entity, QWidget* parent = nullptr);
        ~ErdTableWindow();

    protected:
        bool resolveCreateTableStatement();
        void applyInitialTab();

    private:
        ErdEntity* entity = nullptr;

    signals:
        void entityModified(ErdEntity* entity);
        void changeCreated(ErdChange* change);

    public slots:
        void changesSuccessfullyCommitted();

    protected slots:
        void executeStructureChanges();
};

#endif // ERDTABLEWINDOW_H
