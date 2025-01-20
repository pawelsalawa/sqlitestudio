#ifndef ERDCHANGEREGISTRY_H
#define ERDCHANGEREGISTRY_H

#include <QObject>

class ErdChange;

class ErdChangeRegistry : public QObject
{
    Q_OBJECT

    public:
        explicit ErdChangeRegistry(QObject *parent = nullptr);

        void compact();
        void addChange(ErdChange* change);
        int getPandingChangesCount() const;
        QList<ErdChange*> getEffectiveChanges() const;
        ErdChange* undo();
        ErdChange* redo();
        ErdChange* peekUndo() const;
        ErdChange* peekRedo() const;

    private:
        static constexpr int UNDO_LIMIT = 1000;

        void notifyChangesUpdated();

        QList<ErdChange*> changes;
        int currentIndex = -1; // points last element, until undo is done

    signals:
        void effectiveChangeCountUpdated(int count);

};

#endif // ERDCHANGEREGISTRY_H
