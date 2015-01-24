#include "extralicensemanager.h"
#include <QDebug>
#include <QFile>

ExtraLicenseManager::ExtraLicenseManager()
{
}

ExtraLicenseManager::~ExtraLicenseManager()
{
    for (License* lic : licenses.values())
        delete lic;

    licenses.clear();
}

bool ExtraLicenseManager::addLicense(const QString& title, const QString& filePath)
{
    return addLicense(title, filePath, Type::FILE);
}

bool ExtraLicenseManager::addLicenseContents(const QString& title, const QString& contents)
{
    return addLicense(title, contents, Type::CONTENTS);
}

void ExtraLicenseManager::setViolatedLicense(const QString& title, const QString& violationMessage)
{
    if (!licenses.contains(title))
        return;

    License* lic = licenses[title];
    lic->violated = true;
    lic->violationMessage = violationMessage;
}

void ExtraLicenseManager::unsetViolatedLicense(const QString& title)
{
    if (!licenses.contains(title))
        return;

    License* lic = licenses[title];
    lic->violated = false;
    lic->violationMessage = QString();
}

bool ExtraLicenseManager::isViolatedLicense(const QString& title)
{
    if (!licenses.contains(title))
        return false;

    return licenses[title]->violated;
}

QString ExtraLicenseManager::getViolationMessage(const QString& title)
{
    if (!licenses.contains(title))
        return QString::null;

    return licenses[title]->violationMessage;
}

bool ExtraLicenseManager::removeLicense(const QString& title)
{
    if (!licenses.contains(title))
        return false;

    delete licenses[title];
    licenses.remove(title);
    return true;
}

QHash<QString, QString> ExtraLicenseManager::getLicensesContents() const
{
    QHash<QString, QString> result;
    License* lic = nullptr;
    for (const QString& title : licenses.keys())
    {
        lic = licenses[title];
        switch (lic->type)
        {
            case Type::CONTENTS:
                result[title] = lic->data;
                break;
            case Type::FILE:
                result[title] = readLicenseFile(lic->data);
                break;
        }
    }
    return result;
}

bool ExtraLicenseManager::addLicense(const QString& title, const QString& data, ExtraLicenseManager::Type type)
{
    if (licenses.contains(title))
        return false;

    License* lic = new License;
    lic->title = title;
    lic->data = data;
    lic->type = type;
    licenses[title] = lic;
    return true;
}

QString ExtraLicenseManager::readLicenseFile(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Error opening" << file.fileName();
        return QString::null;
    }
    QString contents = QString::fromLatin1(file.readAll());
    file.close();
    return contents;
}
