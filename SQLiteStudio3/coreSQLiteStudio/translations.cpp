#include "translations.h"
#include "sqlitestudio.h"
#include <QTranslator>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

QHash<QString,QTranslator*> SQLITESTUDIO_TRANSLATIONS;
QStringList SQLITESTUDIO_TRANSLATION_DIRS = QStringList({":/msg", ":/msg/translations", "msg", "translations"});

void loadTranslation(const QString& baseName)
{
    if (SQLITESTUDIO_TRANSLATIONS.contains(baseName))
        return;

    QTranslator* translator = new QTranslator();

    QString fName;
    bool res = false;
    QString lang = SQLITESTUDIO->getCurrentLang();
    QStringList filters = QStringList({baseName+"_"+lang+".qm"});
    QDir dir;

    for (const QString& dirPath : SQLITESTUDIO_TRANSLATION_DIRS)
    {
        dir = dirPath;
        for (const QString& f : dir.entryList(filters))
        {
            res = translator->load(f, dirPath);
            if (res)
            {
                fName = dirPath + "/" + f;
                break;
            }
        }

        if (res)
            break;
    }

    if (!res)
        return;

    qApp->installTranslator(translator);
    SQLITESTUDIO_TRANSLATIONS[baseName] = translator;
    qDebug() << "Loaded:" << fName;
}

void unloadTranslation(const QString& baseName)
{
    if (!SQLITESTUDIO_TRANSLATIONS.contains(baseName))
        return;

    QTranslator* trans = SQLITESTUDIO_TRANSLATIONS[baseName];
    SQLITESTUDIO_TRANSLATIONS.remove(baseName);
    qApp->removeTranslator(trans);
    delete trans;
}

void loadTranslations(const QStringList& baseNames)
{
    for (const QString& name : baseNames)
        loadTranslation(name);
}

QStringList getAvailableTranslations()
{
    QSet<QString> locales;
    QRegularExpression re("[^\\_]+\\_(\\w+)\\.qm");
    QRegularExpressionMatch match;
    QDir dir;
    QStringList filters = QStringList({"*_*.qm"});
    for (const QString& dirPath : SQLITESTUDIO_TRANSLATION_DIRS)
    {
        dir = dirPath;
        for (const QString& f : dir.entryList(filters))
        {
            match = re.match(f);
            if (!match.isValid())
                continue;

            locales << match.captured(1).toLower();
        }
    }
    locales << "en";

    return locales.toList();
}

QMap<QString,QString> getAvailableLanguages()
{
    QMap<QString,QString> langs;
    QStringList translations = getAvailableTranslations();
    QLocale locale;
    QString langName;
    for (const QString& trans : translations)
    {
        locale = QLocale(trans);
        langName = locale.nativeLanguageName();
        if (langName.isEmpty())
            langName = trans;

        langs[langName] = trans;
    }

    return langs;
}
