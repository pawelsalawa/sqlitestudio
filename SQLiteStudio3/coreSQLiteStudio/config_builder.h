#ifndef CFGINTERNALS_H
#define CFGINTERNALS_H

#include "coreSQLiteStudio_global.h"
#include <QHash>
#include <QList>
#include <QString>
#include <QVariant>

#define CFG_CATEGORIES(Type,Body) _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,"",QString())

#define CFG_CATEGORY(Name,Body) \
    _CFG_CATEGORY_WITH_TITLE(Name,Body,QString())

#define CFG_ENTRY(Type, Name, ...) CfgTypedEntry<Type> Name = CfgTypedEntry<Type>(#Name, ##__VA_ARGS__);

#define CFG_DEFINE(Type) _CFG_DEFINE(Type, true)
#define CFG_DEFINE_RUNTIME(Type) _CFG_DEFINE(Type, false)
#define CFG_LOCAL(Type, Name) Cfg::Type Name = Cfg::Type(false);
#define CFG_DEFINE_LAZY(Type) \
    namespace Cfg\
    {\
        Type* cfgMainInstance##Type = nullptr;\
        void init##Type##Instance()\
        {\
            cfgMainInstance##Type = new Type(true);\
        }\
        Type* get##Type##Instance()\
        {\
            return cfgMainInstance##Type;\
        }\
        CfgLazyInitializer* cfgMainInstance##Type##LazyInit = new CfgLazyInitializer(init##Type##Instance, #Type);\
    }

#define CFG_INSTANCE(Type) (*Cfg::get##Type##Instance())

class CfgEntry;
class CfgCategory;

class API_EXPORT CfgLazyInitializer
{
    public:
        CfgLazyInitializer(std::function<void(void)> initFunc, const char* name);

        static void init();

    private:
        void doInitialize();

        std::function<void(void)> initFunc;

        static QList<CfgLazyInitializer*> instances;
};

class API_EXPORT CfgMain
{
    friend class CfgCategory;

    public:
        CfgMain(const QString& name, bool persistable, const char* metaName, const QString& title);
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
        const char* getMetaName() const;
        QString getTitle() const;

    private:
        QString name;
        const char* metaName;
        QString title;
        bool persistable = true;
        QHash<QString,CfgCategory*> childs;

        static QList<CfgMain*> instances;
};

class API_EXPORT CfgCategory
{
    friend class CfgEntry;

    public:
        CfgCategory(const QString& name, const QString& title);

        QString toString() const;
        operator QString() const;
        QHash<QString,CfgEntry*>& getEntries();
        void reset();
        void savepoint();
        void restore();
        void release();
        QString getTitle() const;

    private:
        QString name;
        QString title;
        bool persistable = true;
        QHash<QString,CfgEntry*> childs;
};

class API_EXPORT CfgEntry : public QObject
{
        Q_OBJECT

    public:
        typedef QVariant (*DefaultValueProviderFunc)();

        explicit CfgEntry(const CfgEntry& other);
        CfgEntry(const QString& name, const QVariant& defValue, const QString& title);
        virtual ~CfgEntry();

        QVariant get() const;
        QVariant getDefultValue() const;
        void set(const QVariant& value);
        operator QString() const;
        void defineDefaultValueFunction(DefaultValueProviderFunc func);
        QString getFullKey() const;
        QString getTitle() const;
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
        QString title;
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
        CfgTypedEntry(const QString& name, DefaultValueProviderFunc func, const QString& title) :
            CfgEntry(name, QVariant(), title)
        {
            defineDefaultValueFunction(func);
        }

        CfgTypedEntry(const QString& name, const T& defValue, const QString& title) :
            CfgEntry(name, defValue, title) {}

        CfgTypedEntry(const QString& name, DefaultValueProviderFunc func) :
            CfgTypedEntry(name, func, QString())
        {}

        CfgTypedEntry(const QString& name, const T& defValue) :
            CfgTypedEntry(name, QVariant::fromValue(defValue), QString()) {}

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

Q_DECLARE_METATYPE(CfgMain*)
Q_DECLARE_METATYPE(CfgCategory*)
Q_DECLARE_METATYPE(CfgEntry*)


#define _CFG_CATEGORIES_WITH_METANAME(Type,Body,MetaName) \
    _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,MetaName,QString())

#define _CFG_CATEGORIES_WITH_TITLE(Type,Body,Title) \
    _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,"",Title)

#define _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,MetaName,Title) \
    namespace Cfg\
    {\
        struct API_EXPORT Type : public CfgMain\
        {\
            Type(bool persistable) : CfgMain(#Type, persistable, MetaName, Title) {}\
            Body\
        };\
        API_ONLY_CORE_EXPORT Type* get##Type##Instance();\
    }

#define _CFG_DEFINE(Type, Persistant) \
    namespace Cfg\
    {\
        Type* cfgMainInstance##Type = new Type(Persistant);\
        Type* get##Type##Instance()\
        {\
            return cfgMainInstance##Type;\
        }\
    }

#define _CFG_CATEGORY_WITH_TITLE(Name,Body,Title) \
    struct API_EXPORT _##Name##Type : public CfgCategory\
    {\
        _##Name##Type() : CfgCategory(#Name, Title) {}\
        Body\
    };\
    _##Name##Type Name;


#endif // CFGINTERNALS_H
