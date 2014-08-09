#ifndef EXTACTIONPROTOTYPE_H
#define EXTACTIONPROTOTYPE_H

#include <QString>
#include <QIcon>
#include <QObject>

class QAction;
class ExtActionContainer;

class ExtActionPrototype : public QObject
{
        Q_OBJECT

        friend class ExtActionContainer;

    public:
        explicit ExtActionPrototype(QObject* parent);
        ExtActionPrototype(const QString& text, QObject* parent = 0);
        ExtActionPrototype(const QIcon& icon, const QString& text, QObject* parent = 0);
        ~ExtActionPrototype();

        QString text() const;
        QAction* create(QObject* parent = 0);

    private:
        void emitInsertedTo(ExtActionContainer* actionContainer, int toolbar, QAction* action);
        void emitAboutToRemoveFrom(ExtActionContainer* actionContainer, int toolbar, QAction* action);
        void emitRemovedFrom(ExtActionContainer* actionContainer, int toolbar, QAction* action);
        void emitTriggered(ExtActionContainer* actionContainer, int toolbar);

        QIcon icon;
        QString actionText;
        bool separator = false;

    signals:
        void insertedTo(ExtActionContainer* actionContainer, int toolbar, QAction* action);
        void aboutToRemoveFrom(ExtActionContainer* actionContainer, int toolbar, QAction* action);
        void removedFrom(ExtActionContainer* actionContainer, int toolbar, QAction* action);
        void triggered(ExtActionContainer* actionContainer, int toolbar);
};

#endif // EXTACTIONPROTOTYPE_H
