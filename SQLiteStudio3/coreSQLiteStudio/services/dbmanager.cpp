#include "dbmanager.h"
#include <QFileInfo>

DbManager::DbManager(QObject *parent) :
    QObject(parent)
{
}

DbManager::~DbManager()
{
}

QString DbManager::generateDbName(const QString &filePath)
{
    QFileInfo fi(filePath);
    return fi.completeBaseName();
}
