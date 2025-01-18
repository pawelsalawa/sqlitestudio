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

    private:
        static constexpr int UNDO_LIMIT = 1000;

        QList<ErdChange*> changes;
        int currentIndex = -1; // points last element, until undo is done

};

#endif // ERDCHANGEREGISTRY_H
