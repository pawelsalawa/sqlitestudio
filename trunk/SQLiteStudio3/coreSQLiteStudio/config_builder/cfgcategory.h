#ifndef CFGCATEGORY_H
#define CFGCATEGORY_H

#include "coreSQLiteStudio_global.h"
#include <QVariant>
#include <QHash>
#include <QString>
#include <QObject>

class CfgEntry;
class CfgMain;

class API_EXPORT CfgCategory : public QObject
{
    Q_OBJECT

    friend class CfgEntry;

    public:
        CfgCategory(const CfgCategory& other);
        CfgCategory(const QString& name, const QString& title);

        QString toString() const;
        operator QString() const;
        QHash<QString,CfgEntry*>& getEntries();
        void reset();
        void savepoint(bool transaction = false);
        void restore();
        void release();
        void commit();
        void rollback();
        void begin();
        QString getTitle() const;
        CfgMain* getMain() const;
        operator CfgCategory*();

    private:
        QString name;
        QString title;
        CfgMain* cfgParent = nullptr;
        bool persistable = true;
        QHash<QString,CfgEntry*> childs;

    private slots:
        void handleEntryChanged();

    signals:
        void changed(CfgEntry* entry);
};

Q_DECLARE_METATYPE(CfgCategory*)

#endif // CFGCATEGORY_H
