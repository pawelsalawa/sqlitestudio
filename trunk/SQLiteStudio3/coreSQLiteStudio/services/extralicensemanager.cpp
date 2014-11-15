#include "extralicensemanager.h"

ExtraLicenseManager::ExtraLicenseManager()
{
}

bool ExtraLicenseManager::addLicense(const QString& title, const QString& filePath)
{
    if (licenses.contains(title))
        return false;

    licenses[title] = filePath;
    return true;
}

bool ExtraLicenseManager::removeLicense(const QString& title)
{
    if (!licenses.contains(title))
        return false;

    licenses.remove(title);
    return true;
}

const QHash<QString, QString>&ExtraLicenseManager::getLicenses() const
{
    return licenses;
}
