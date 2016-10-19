#ifndef CFGMAIN_H
#define CFGMAIN_H

#include "coreSQLiteStudio_global.h"
#include <QVariant>
#include <QList>
#include <QHash>
#include <QString>

class CfgCategory;
class CfgEntry;

class API_EXPORT CfgMain
{
    friend class CfgCategory;

    public:
        CfgMain(const QString& name, bool persistable, const char* metaName, const QString& title);
        ~CfgMain();

        static void staticInit();
        static QList<CfgMain*> getInstances();
        static QList<CfgMain*> getPersistableInstances();
        static CfgCategory* getCategoryByName(const QString& name);
        static CfgEntry* getEntryByName(const QString& categoryName, const QString& name);
        static CfgEntry* getEntryByPath(const QString& path);

        QHash<QString,CfgCategory*>& getCategories();
        void translateTitle();
        void reset();
        void savepoint(bool transaction = false);
        void restore();
        void release();
        void begin();
        void commit();
        void rollback();
        QStringList getPaths() const;
        QList<CfgEntry*> getEntries() const;

        bool isPersistable() const;
        QString getName() const;
        const char* getMetaName() const;
        QString getTitle() const;
        operator CfgMain*();

    private:
        QString name;
        const char* metaName;
        QString title;
        bool persistable = true;
        QHash<QString,CfgCategory*> childs;

        static QList<CfgMain*>* instances;
};

Q_DECLARE_METATYPE(CfgMain*)

#endif // CFGMAIN_H
