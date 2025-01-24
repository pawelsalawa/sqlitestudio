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

        QString getQuitUncommittedConfirmMessage() const;

    protected:
        bool resolveCreateTableStatement();
        void applyInitialTab();
        void defineCurrentContextDb();

    private:
        ErdEntity* entity = nullptr;

    signals:
        void changeCreated(ErdChange* change);

    public slots:
        void changesSuccessfullyCommitted();
        bool commitStructure(bool skipWarning = false);
        void rollbackStructure();

    protected slots:
        void executeStructureChanges();
};

#endif // ERDTABLEWINDOW_H
