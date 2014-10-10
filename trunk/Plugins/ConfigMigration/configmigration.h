#ifndef CONFIGMIGRATION_H
#define CONFIGMIGRATION_H

#include "configmigration_global.h"
#include "plugins/generalpurposeplugin.h"
#include "plugins/genericplugin.h"

class CONFIGMIGRATIONSHARED_EXPORT ConfigMigration : public GenericPlugin, public GeneralPurposePlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("configmigration.json")

    public:
        ConfigMigration();

        bool init();

    private:
        QString findOldConfig();
        bool checkOldDir(const QString& dir, QString& output);

        static const constexpr char* ACTION_LINK = "migrateOldConfig";

    private slots:
        void linkActivated(const QString& link);
};

#endif // CONFIGMIGRATION_H
