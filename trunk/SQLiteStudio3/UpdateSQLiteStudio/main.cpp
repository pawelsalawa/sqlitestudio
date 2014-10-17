#include "services/updatemanager.h"
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString path = app.applicationDirPath() + QLatin1Char('/') + UpdateManager::WIN_INSTALL_FILE;
    QFile installFile(path);
    if (QFileInfo(path).isReadable())
    {
        installFile.open(QIODevice::ReadOnly);
        QTextStream inStr(&installFile);
        QString option = inStr.readLine();
        QString backupDir = inStr.readLine();
        QString appDir = inStr.readLine();
        installFile.close();
        installFile.remove();

        QString tempDir = app.applicationDirPath();
        if (option == UpdateManager::UPDATE_OPTION_NAME)
        {
            bool res = UpdateManager::executeFinalStep(tempDir, backupDir, appDir);
            if (res)
            {
                QFile doneFile(appDir + QLatin1Char('/') + UpdateManager::WIN_UPDATE_DONE_FILE);
                doneFile.open(QIODevice::WriteOnly);
                doneFile.close();
            }
            else
                qCritical() << QString("Could not execute final step with root priviledges: %1").arg(UpdateManager::getStaticErrorMessage());
        }
        else
        {
            qCritical() << QString("Option passed to updater not matched: '%1' != '%2'").arg(option, UpdateManager::UPDATE_OPTION_NAME);
        }
    }
    else
    {
        qCritical() << QString("Updater installation file (%1) was not readable.").arg(path);
    }

    return 0;
}
