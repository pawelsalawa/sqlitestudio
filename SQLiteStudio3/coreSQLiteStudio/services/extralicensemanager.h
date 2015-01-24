#ifndef EXTRALICENSEMANAGER_H
#define EXTRALICENSEMANAGER_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QHash>

class API_EXPORT ExtraLicenseManager
{
    public:
        enum class Type
        {
            FILE,
            CONTENTS
        };

        struct License
        {
            QString title;
            QString data;
            Type type;
            QString violationMessage;
            bool violated = false;
        };

        ExtraLicenseManager();
        virtual ~ExtraLicenseManager();

        bool addLicense(const QString& title, const QString& filePath);
        bool addLicenseContents(const QString& title, const QString& contents);
        void setViolatedLicense(const QString& title, const QString& violationMessage);
        void unsetViolatedLicense(const QString& title);
        bool isViolatedLicense(const QString& title);
        QString getViolationMessage(const QString& title);
        bool removeLicense(const QString& title);
        QHash<QString,QString> getLicensesContents() const;

    private:
        bool addLicense(const QString& title, const QString& data, Type type);
        QString readLicenseFile(const QString& path) const;

        QHash<QString,License*> licenses;
};

#endif // EXTRALISENCEMANAGER_H
