#ifndef SCRIPTINGQTDBPROXY_H
#define SCRIPTINGQTDBPROXY_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QVariant>
#include <QJSValue>

class Db;

class ScriptingQtDbProxy : public QObject
{
        Q_OBJECT
    public:
        explicit ScriptingQtDbProxy(QJSEngine* engine, QObject *parent = 0);

        Db* getDb() const;
        void setDb(Db* value);

        bool getUseDbLocking() const;
        void setUseDbLocking(bool value);

    private:
        QVariant evalInternal(const QString& sql, const QList<QVariant>& listArgs, const QMap<QString, QVariant>& mapArgs, bool singleCell,
                              const QJSValue* funcPtr = nullptr);
        QVariant evalInternalErrorResult(bool singleCell);

        static QHash<QString, QVariant> mapToHash(const QMap<QString, QVariant>& map);

        Db* db = nullptr;
        bool useDbLocking = false;
        QJSEngine* engine = nullptr;

    public slots:
        QVariant eval(const QString& sql);
        QVariant eval(const QString& sql, const QList<QVariant>& args);
        QVariant eval(const QString& sql, const QMap<QString, QVariant>& args);
        QVariant eval(const QString& sql, const QList<QVariant>& args, const QJSValue& func);
        QVariant eval(const QString& sql, const QMap<QString, QVariant>& args, const QJSValue& func);
        QVariant onecolumn(const QString& sql, const QList<QVariant>& args);
        QVariant onecolumn(const QString& sql, const QMap<QString, QVariant>& args);
};

#endif // SCRIPTINGQTDBPROXY_H
