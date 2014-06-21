#ifndef GENERICEXPORTPLUGIN_H
#define GENERICEXPORTPLUGIN_H

#include "exportplugin.h"
#include "genericplugin.h"

class API_EXPORT GenericExportPlugin : virtual public GenericPlugin, public ExportPlugin
{
    public:
        void initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config);
        ExportManager::ExportModes getSupportedModes() const;
        CfgMain* getConfig();
        QString getConfigFormName(ExportManager::ExportMode exportMode) const;
        QString getMimeType() const;
        void setExportMode(ExportManager::ExportMode exportMode);
        bool afterExportTable();
        bool beforeExportTables();
        bool afterExportTables();
        bool beforeExportIndexes();
        bool afterExportIndexes();
        bool beforeExportTriggers();
        bool afterExportTriggers();
        bool beforeExportViews();
        bool afterExportViews();
        bool afterExportDatabase();

    protected:
        virtual void initBeforeExport();
        void write(const QString& str);
        void writeln(const QString& str);

        Db* db = nullptr;
        QIODevice* output = nullptr;
        const ExportManager::StandardExportConfig* config = nullptr;
        QTextCodec* codec = nullptr;
        ExportManager::ExportMode exportMode = ExportManager::UNDEFINED;
};

#endif // GENERICEXPORTPLUGIN_H
