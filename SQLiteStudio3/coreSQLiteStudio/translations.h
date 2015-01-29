#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QMap>

API_EXPORT void loadTranslations(const QStringList& baseNames);
API_EXPORT void loadTranslation(const QString& baseName);
API_EXPORT void unloadTranslation(const QString& baseName);
API_EXPORT void setDefaultLanguage(const QString& lang);
API_EXPORT QString getConfigLanguageDefault();

/**
 * @brief Provides list of translations as code names.
 * @return List of available translations in their code names (pl, pt, de, ...).
 */
API_EXPORT QStringList getAvailableTranslations();

/**
 * @brief Provides list of languages and their code names.
 * @return A map of pairs, there key is a translated (with a current language) name of language and value is its translation code name (pl, pt, de, ...).
 *
 * As the result is a QMap, it comes sorted by a translated names of languages.
 */
API_EXPORT QMap<QString, QString> getAvailableLanguages();

#endif // TRANSLATIONS_H
