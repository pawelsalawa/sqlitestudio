#ifndef ERDTABLEWINDOW_H
#define ERDTABLEWINDOW_H

#include "windows/tablewindow.h"
#include <QObject>

class ErdEntity;

class ErdTableWindow : public TableWindow
{
        Q_OBJECT

    public:
        ErdTableWindow(Db* db, ErdEntity* entity, QWidget* parent = nullptr);
        ~ErdTableWindow();

    protected:
        bool resolveCreateTableStatement();
        bool resolveOriginalCreateTableStatement();
        void applyInitialTab();

    private:
        ErdEntity* entity = nullptr;

    signals:

    protected slots:
        void executeStructureChanges();
};

#endif // ERDTABLEWINDOW_H
