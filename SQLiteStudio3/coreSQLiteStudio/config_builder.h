#ifndef CFGINTERNALS_H
#define CFGINTERNALS_H

#include "config_builder/cfgmain.h"
#include "config_builder/cfgcategory.h"
#include "config_builder/cfgentry.h"
#include "config_builder/cfglazyinitializer.h"

#define CFG_CATEGORIES(Type,Body) _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,"",QString(),API_EXPORT)

#define CFG_CATEGORY(Name,Body) \
    _CFG_CATEGORY_WITH_TITLE(Name,Body,QString())

#define CFG_ENTRY(Type, Name, ...) CfgTypedEntry<Type> Name = CfgTypedEntry<Type>(#Name, ##__VA_ARGS__);

#define CFG_DEFINE(Type) _CFG_DEFINE(Type, true)
#define CFG_DEFINE_RUNTIME(Type) _CFG_DEFINE(Type, false)
#define CFG_LOCAL(Type, Name) Cfg::Type Name = Cfg::Type(false);
#define CFG_LOCAL_PERSISTABLE(Type, Name) Cfg::Type Name = Cfg::Type(true);
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

#define CFG_DELETE_INSTANCE(Type) \
    if (Cfg::cfgMainInstance##Type) \
        delete Cfg::cfgMainInstance##Type; \
    Cfg::cfgMainInstance##Type = nullptr;


// Macros below are kind of private. You should not need to use them explicitly.
// They are called from macros above.

#define _CFG_CATEGORIES_WITH_METANAME(Type,Body,MetaName) \
    _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,MetaName,QString(),API_EXPORT)

#define _CFG_CATEGORIES_WITH_TITLE(Type,Body,Title) \
    _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,"",Title,API_EXPORT)

#define _CFG_CATEGORIES_WITH_METANAME_AND_TITLE(Type,Body,MetaName,Title,ExportType) \
    namespace Cfg\
    {\
        struct ExportType Type : public CfgMain\
        {\
            Type(bool persistable) : CfgMain(#Type, persistable, MetaName, Title) {}\
            Body\
        };\
        ExportType Type* get##Type##Instance();\
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
