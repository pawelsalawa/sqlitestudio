#ifndef ERDCHANGEREGISTRY_H
#define ERDCHANGEREGISTRY_H

#include <QObject>

class ErdChange;

class ErdChangeRegistry : public QObject
{
    Q_OBJECT

    public:
        explicit ErdChangeRegistry(QObject *parent = nullptr);

        QList<ErdChange*> compactedEffectiveChanges();
        void addChange(ErdChange* change);
        int getPendingChangesCount() const;
        QList<ErdChange*> getPendingChanges(bool includeNonDdl = false) const;
        ErdChange* undo();
        ErdChange* redo();
        ErdChange* peekUndo() const;
        ErdChange* peekRedo() const;
        bool isUndoAvailable() const;
        bool isRedoAvailable() const;
        void clear();

    private:
        static constexpr int UNDO_LIMIT = 1000;

        void notifyChangesUpdated();
        void notifyUndoRedoState();

        QList<ErdChange*> changes;
        int currentIndex = -1; // points last element, until undo is done

    signals:
        void effectiveChangeCountUpdated(int count, bool undoAvailable, bool redoAvailable);

};

#endif // ERDCHANGEREGISTRY_H
