#ifndef CFGENTRY_H
#define CFGENTRY_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QVariant>
#include <QObject>

class CfgCategory;
class CfgMain;

class API_EXPORT CfgEntry : public QObject
{
        Q_OBJECT

    friend class CfgCategory;

    public:
        typedef QVariant (*DefaultValueProviderFunc)();

        explicit CfgEntry(const CfgEntry& other);
        CfgEntry(const QString& name, const QVariant& defValue, const QString& title);
        virtual ~CfgEntry();

        QVariant get() const;
        QVariant getDefaultValue() const;
        void set(const QVariant& value);
        operator QString() const;
        void defineDefaultValueFunction(DefaultValueProviderFunc func);
        QString getFullKey() const;
        QString getTitle() const;
        QString getName() const;
        void translateTitle();
        void reset();
        bool isPersistable() const;
        bool isPersisted() const;
        void savepoint(bool transaction = false);
        void begin();
        void restore();
        void release();
        void commit();
        void rollback();
        CfgCategory* getCategory() const;
        CfgMain* getMain() const;

        /**
         * @brief operator CfgEntry *
         *
         * Allows implict casting from value object into pointer. It simply returns "this".
         * It's useful to use config objects directly in QObject::connect() arguments,
         * cause it accepts pointers, not values, but CfgEntry is usually accessed by value.
         */
        operator CfgEntry*();

    protected:
        bool persistable = true;
        CfgCategory* parent = nullptr;
        QString name;
        QVariant defValue;
        QString title;
        QVariant backup;
        bool transaction = false;
        mutable bool cached = false;
        mutable QVariant cachedValue;
        DefaultValueProviderFunc defValueFunc = nullptr;

    signals:
        void changed(const QVariant& newValue);
        void persisted(const QVariant& newValue);
};

template <class T>
class CfgTypedEntry : public CfgEntry
{
    public:
        CfgTypedEntry(const QString& name, DefaultValueProviderFunc func, const QString& title) :
            CfgEntry(name, QVariant(), title)
        {
            defineDefaultValueFunction(func);
        }

        CfgTypedEntry(const QString& name, const T& defValue, const QString& title) :
            CfgEntry(name, QVariant::fromValue(defValue), title) {}

        CfgTypedEntry(const QString& name, DefaultValueProviderFunc func) :
            CfgTypedEntry(name, func, QString()) {}

        CfgTypedEntry(const QString& name, const T& defValue, bool persistable) :
            CfgTypedEntry(name, defValue, QString())
        {
            this->persistable = persistable;
        }

        CfgTypedEntry(const QString& name, DefaultValueProviderFunc func, bool persistable) :
            CfgTypedEntry(name, func, QString())
        {
            this->persistable = persistable;
        }

        CfgTypedEntry(const QString& name, const T& defValue) :
            CfgTypedEntry(name, defValue, QString()) {}

        CfgTypedEntry(const QString& name) :
            CfgEntry(name, QVariant(), QString()) {}

        CfgTypedEntry(const CfgTypedEntry& other) :
            CfgEntry(other) {}

        T get()
        {
            QVariant v = CfgEntry::get();
            return v.value<T>();
        }

        void set(const T& value)
        {
            CfgEntry::set(QVariant::fromValue<T>(value));
        }
};

typedef CfgTypedEntry<QString> CfgStringEntry;

Q_DECLARE_METATYPE(CfgEntry*)

#endif // CFGENTRY_H
