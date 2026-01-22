#ifndef ERDTABLEWINDOW_H
#define ERDTABLEWINDOW_H

#include "windows/tablewindow.h"
#include "erdpropertiespanel.h"
#include <QObject>

class ErdEntity;
class ErdChange;

class ErdTableWindow : public TableWindow, public ErdPropertiesPanel
{
        Q_OBJECT

    public:
        ErdTableWindow(Db* db, ErdEntity* entity, QWidget* parent = nullptr);
        ~ErdTableWindow();

        QString getQuitUncommittedConfirmMessage() const;
        bool commitErdChange();
        void abortErdChange();
        bool editColumn(const QString& columnName);
        ErdEntity* getEntity() const;

    protected:
        bool resolveOriginalCreateTableStatement();
        bool resolveCreateTableStatement();
        void applyInitialTab();
        void defineCurrentContextDb();

    private:
        bool handleFailedStructureChanges(bool skipWarning);
        void setErrorRecording(bool enabled);

        ErdEntity* entity = nullptr;
        QStringList recordedErrors;
        QString originalContent;

    public slots:
        void changesSuccessfullyCommitted();
        bool commitStructure(bool skipWarning = false);
        void rollbackStructure();
        void nameEditedInline(const QString& newName);
        void columnEditedInline(int columnIdx, const QString& newName);
        void columnDeletedInline(int columnIdx);

    protected slots:
        void executeStructureChanges();
        void errorRecorded(const QString& msg);

    signals:
        void changeCreated(ErdChange* change);
        void editedEntityShouldBeDeleted(ErdEntity* entity);
        void requestReEditForEntity(ErdEntity* entity);
};

#endif // ERDTABLEWINDOW_H
