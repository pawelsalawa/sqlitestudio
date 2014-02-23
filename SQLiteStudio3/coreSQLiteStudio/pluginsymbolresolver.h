#ifndef PLUGINSYMBOLRESOLVER_H
#define PLUGINSYMBOLRESOLVER_H

#include "coreSQLiteStudio_global.h"
#include <QStringList>
#include <QLibrary>

class API_EXPORT PluginSymbolResolver
{
    public:
        PluginSymbolResolver();

        void addFileNameMask(const QString& mask);
        void addLookupSubFolder(const QString& name);
        bool load();
        QFunctionPointer resolve(const char* symbol);

    private:
        QStringList nameFilters;
        QStringList subFolders;
        QLibrary lib;
};

#endif // PLUGINSYMBOLRESOLVER_H
