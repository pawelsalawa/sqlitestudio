#include "translations.h"
#include "sqlitestudio.h"
#include <QTranslator>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

QHash<QString,QTranslator*> SQLITESTUDIO_TRANSLATIONS;
QStringList SQLITESTUDIO_TRANSLATION_DIRS = QStringList({"msg", "translations", ":/msg", ":/msg/translations"});

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

    for (QString& dirPath : SQLITESTUDIO_TRANSLATION_DIRS)
    {
        dir.setPath(dirPath);
        for (QString& f : dir.entryList(filters))
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
    {
        delete translator;
        return;
    }

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
    static QRegularExpression re("[^\\_]+\\_(\\w+)\\.qm");
    QSet<QString> locales;
    QRegularExpressionMatch match;
    QDir dir;
    QStringList filters = QStringList({"*_*.qm"});
    for (QString& dirPath : SQLITESTUDIO_TRANSLATION_DIRS)
    {
        dir.setPath(dirPath);
        for (QString& f : dir.entryList(filters))
        {
            match = re.match(f);
            if (!match.isValid())
                continue;

            locales << match.captured(1).toLower();
        }
    }
    locales << "en";

    // #4278 - the en_us translation as explicit qm file is unnecessary,
    // but produced by CrowdIn. The "en" is default and used for American English.
    locales.remove("en_us");

    return locales.values();
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

void setDefaultLanguage(const QString& lang)
{
    CFG_CORE.General.Language.set(lang);
}

QString getConfigLanguageDefault()
{
    return CFG_CORE.General.Language.getDefaultValue().toString();
}
