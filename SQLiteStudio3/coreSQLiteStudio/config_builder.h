#ifndef CFGINTERNALS_H
#define CFGINTERNALS_H

#include "coreSQLiteStudio_global.h"
#include <QHash>
#include <QList>
#include <QString>
#include <QVariant>

#define CFG_CATEGORY(Name,Body) \
    struct API_EXPORT _##Name##Type : public CfgCategory\
    {\
        _##Name##Type() : CfgCategory(#Name) {}\
        Body\
    };\
    _##Name##Type Name;

#define CFG_CATEGORIES(Type,Body) \
    namespace Cfg\
    {\
        struct API_EXPORT Type : public CfgMain\
        {\
            Type(bool persistable) : CfgMain(#Type, persistable) {}\
            Body\
        };\
        API_ONLY_CORE_EXPORT Type* get##Type##Instance();\
    }

#define CFG_ENTRY(Type, Name, ...) CfgTypedEntry<Type> Name = CfgTypedEntry<Type>(#Name, ##__VA_ARGS__);

#define CFG_DEFINE(Type) _CFG_DEFINE(Type, true)
#define CFG_DEFINE_RUNTIME(Type) _CFG_DEFINE(Type, false)
#define CFG_LOCAL(Type, Name) Cfg::Type Name = Cfg::Type(false);

#define _CFG_DEFINE(Type, Persistant) \
    namespace Cfg\
    {\
        Type* cfgMainInstance##Type = new Type(Persistant);\
        Type* get##Type##Instance()\
        {\
            return cfgMainInstance##Type;\
        }\
    }

#define CFG_INSTANCE(Type) (*Cfg::get##Type##Instance())

class CfgEntry;
class CfgCategory;

class API_EXPORT CfgMain
{
    friend class CfgCategory;

    public:
        explicit CfgMain(const QString& name, bool persistable = true);
        ~CfgMain();

        static void staticInit();
        static QList<CfgMain*> getInstances();
        static QList<CfgMain*> getPersistableInstances();

        QHash<QString,CfgCategory*>& getCategories();
        void reset();
        void savepoint();
        void restore();
        void release();

        bool isPersistable() const;
        QString getName() const;

    private:
        QString name;
        bool persistable = true;
        QHash<QString,CfgCategory*> childs;

        static QList<CfgMain*> instances;
};

class API_EXPORT CfgCategory
{
    friend class CfgEntry;

    public:
        explicit CfgCategory(const QString& name);

        QString toString() const;
        operator QString() const;
        QHash<QString,CfgEntry*>& getEntries();
        void reset();
        void savepoint();
        void restore();
        void release();

    private:
        bool persistable = true;
        QHash<QString,CfgEntry*> childs;
        QString name;
};

class API_EXPORT CfgEntry : public QObject
{
        Q_OBJECT

    public:
        typedef QVariant (*DefaultValueProviderFunc)();

        explicit CfgEntry(const CfgEntry& other);
        CfgEntry(const QString& name, const QVariant& defValue);
        virtual ~CfgEntry();

        QVariant get() const;
        QVariant getDefultValue() const;
        void set(const QVariant& value);
        operator QString() const;
        void defineDefaultValueFunction(DefaultValueProviderFunc func);
        QString getFullKey() const;
        void reset();
        bool isPersistable() const;
        bool isPersisted() const;
        void savepoint();
        void restore();
        void release();

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
        CfgCategory* parent;
        QString name;
        QVariant defValue;
        QVariant backup;
        mutable bool cached = false;
        mutable QVariant cachedValue;
        DefaultValueProviderFunc defValueFunc = nullptr;

    signals:
        void changed(const QVariant& newValue);
};

template <class T>
class CfgTypedEntry : public CfgEntry
{
    public:
        CfgTypedEntry(const QString& name, DefaultValueProviderFunc func) :
            CfgEntry(name, QVariant())
        {
            defineDefaultValueFunction(func);
        }

        CfgTypedEntry(const QString& name, const T& defValue) :
            CfgEntry(name, QVariant::fromValue(defValue)) {}

        CfgTypedEntry(const QString& name) :
            CfgEntry(name, QVariant()) {}

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

Q_DECLARE_METATYPE(CfgMain*)
Q_DECLARE_METATYPE(CfgCategory*)
Q_DECLARE_METATYPE(CfgEntry*)

#endif // CFGINTERNALS_H
