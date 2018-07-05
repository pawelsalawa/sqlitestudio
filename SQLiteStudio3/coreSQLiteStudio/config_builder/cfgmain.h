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

        /**
         * @brief Accepts QVariant produced by toQVariant().
         *
         * This method assumes that the QVariant is actually a multi-level QHash
         * produced by toQVariant() method.
         * It sets all values recursivly using values from provided QVariant.
         */
        void setValuesFromQVariant(const QVariant& cfgMainHash);

        bool isPersistable() const;
        QString getName() const;
        const char* getMetaName() const;
        QString getTitle() const;
        operator CfgMain*();

        /**
         * @brief Serializes this CfgMain to recursive QHash.
         * @return Recursive QHash, where top level has one entry (name of CfgMain), then next level has keys as CfgCategory names, and last one has keys as CfgEntry names.
         */
        QVariant toQVariant() const;

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
