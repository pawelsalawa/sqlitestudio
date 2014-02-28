#ifndef CFGINTERNALS_H
#define CFGINTERNALS_H

#include "coreSQLiteStudio_global.h"
#include <QHash>
#include <QList>
#include <QString>
#include <QVariant>

class CfgEntry;
class CfgCategory;

class API_EXPORT CfgMain
{
    friend class CfgCategory;

    public:
        explicit CfgMain(const QString& name);

        static void staticInit();
        static QList<CfgMain*> getInstances();

        QHash<QString,CfgCategory*>& getCategories();

    private:
        QString name;
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

    private:
        QHash<QString,CfgEntry*> childs;
        QString name;
};

class API_EXPORT CfgEntry : public QObject
{
        Q_OBJECT

    public:
        typedef QVariant (*DefaultValueProviderFunc)();

        explicit CfgEntry(const CfgEntry& other);
        CfgEntry(const QString& name, const QString &dbKey, const QVariant& defValue);
        virtual ~CfgEntry();

        QVariant get() const;
        QVariant getDefultValue() const;
        void set(const QVariant& value);
        operator QString() const;
        void defineDefaultValueFunction(DefaultValueProviderFunc func);
        QString getFullDbKey() const;
        QString getFullSymbolicKey() const;

        /**
         * @brief operator const CfgEntry *
         * Allows implict casting from value object into pointer. It simply returns "this".
         * It's useful to use config objects directly in QObject::connect() arguments,
         * cause it accepts pointers, not values, but CfgEntry is usually accessed by value.
         */
        operator const CfgEntry*() const;

    protected:
        CfgCategory* parent;
        QString name;
        QString dbKey;
        QVariant defValue;
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
        CfgTypedEntry(const QString& name, const QString &value, DefaultValueProviderFunc func) :
            CfgEntry(name, value, QVariant())
        {
            defineDefaultValueFunction(func);
        }

        CfgTypedEntry(const QString& name, const QString &value, const T& defValue) :
            CfgEntry(name, value, defValue) {}

        CfgTypedEntry(const QString& name, const QString &value) :
            CfgEntry(name, value, QVariant()) {}

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
            Type() : CfgMain(#Type) {}\
            Body\
        };\
    }

#define CFG_ENTRY(Type, Name, ...) CfgTypedEntry<Type> Name = CfgTypedEntry<Type>(#Name, #Name, ##__VA_ARGS__);

#define CFG_DECLARE(Type) \
    namespace Cfg\
    {\
        API_ONLY_CORE_EXPORT Type* get##Type##Instance();\
    }
//extern Type* cfgMainInstance##Type;

#define CFG_DEFINE(Type) \
    namespace Cfg\
    {\
        Type* cfgMainInstance##Type = get##Type##Instance();\
        Type* get##Type##Instance()\
        {\
            if (!cfgMainInstance##Type)\
                cfgMainInstance##Type = new Type();\
        \
            return cfgMainInstance##Type;\
        }\
    }

#define CFG_INSTANCE(Type) (*Cfg::get##Type##Instance())

Q_DECLARE_METATYPE(CfgMain*)
Q_DECLARE_METATYPE(CfgCategory*)
Q_DECLARE_METATYPE(CfgEntry*)

#endif // CFGINTERNALS_H
