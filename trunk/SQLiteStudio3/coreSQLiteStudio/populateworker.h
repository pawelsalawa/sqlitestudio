#ifndef POPULATEWORKER_H
#define POPULATEWORKER_H

#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QStringList>

class Db;
class PopulateEngine;

class PopulateWorker : public QObject, public QRunnable
{
        Q_OBJECT
    public:
        explicit PopulateWorker(Db* db, const QString& table, const QStringList& columns, const QList<PopulateEngine*>& engines, qint64 rows, QObject *parent = 0);
        ~PopulateWorker();

        void run();

    private:
        bool isInterrupted();
        bool beforePopulating();
        void afterPopulating();

        Db* db = nullptr;
        QString table;
        QStringList columns;
        QList<PopulateEngine*> engines;
        qint64 rows;
        bool interrupted = false;
        QMutex interruptMutex;

    public slots:
        void interrupt();

    signals:
        void finished(bool result);
};

#endif // POPULATEWORKER_H
