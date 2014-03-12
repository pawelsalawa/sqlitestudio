#include "pluginsymbolresolver.h"
#include <QCoreApplication>
#include <QDir>

PluginSymbolResolver::PluginSymbolResolver()
{
}

void PluginSymbolResolver::addFileNameMask(const QString &mask)
{
    nameFilters << mask;
}

void PluginSymbolResolver::addLookupSubFolder(const QString &name)
{
    subFolders << name;
}

bool PluginSymbolResolver::load()
{
    QStringList paths = qApp->libraryPaths();
    foreach (QString path, paths)
        foreach (QString subFolder, subFolders)
            paths << path + "/" + subFolder;

    foreach (QString path, paths)
    {
        QDir dir(path);
        foreach (QString file, dir.entryList(nameFilters))
        {
            lib.setFileName(path+"/"+file);
            if (lib.load())
                break;
        }
        if (lib.isLoaded())
            break;
    }

    return lib.isLoaded();
}

QFunctionPointer PluginSymbolResolver::resolve(const char *symbol)
{
    return lib.resolve(symbol);
}
