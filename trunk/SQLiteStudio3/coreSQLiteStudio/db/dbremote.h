#ifndef DBREMOTE_H
#define DBREMOTE_H

#include "db.h"

#include <QObject>
#include <QVariant>
#include <QList>
#include <QMap>

// TODO implement DbRemote

class DbRemote : public Db
{
    Q_OBJECT

    public:
        ~DbRemote();

        static Db* getInstance(const QString& path, const QString &options = QString::null);

        QString getErrorText();
        int getErrorCode();
        QString getTypeLabel();

    protected:
        DbRemote();

        bool isOpenInternal();
        SqlResultsPtr execInternal(const QString& query, const QList<QVariant>& args,
                                                bool singleCell);
        SqlResultsPtr execInternal(const QString& query, const QMap<QString,QVariant>& args,
                                                bool singleCell);
        bool init();

    public slots:
        bool openQuiet();
        bool closeQuiet();

};

#endif // DBREMOTE_H
