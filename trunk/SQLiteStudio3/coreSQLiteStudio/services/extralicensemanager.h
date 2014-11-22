#ifndef EXTRALICENSEMANAGER_H
#define EXTRALICENSEMANAGER_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QHash>

class API_EXPORT ExtraLicenseManager
{
    public:
        ExtraLicenseManager();

        bool addLicense(const QString& title, const QString& filePath);
        bool removeLicense(const QString& title);
        const QHash<QString,QString>& getLicenses() const;

    private:
        QHash<QString,QString> licenses;
};

#endif // EXTRALISENCEMANAGER_H
