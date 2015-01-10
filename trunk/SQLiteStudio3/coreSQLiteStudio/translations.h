#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include <QString>
#include <QMap>

void loadTranslations(const QStringList& baseNames);
void loadTranslation(const QString& baseName);
void unloadTranslation(const QString& baseName);

/**
 * @brief Provides list of translations as code names.
 * @return List of available translations in their code names (pl, pt, de, ...).
 */
QStringList getAvailableTranslations();

/**
 * @brief Provides list of languages and their code names.
 * @return A map of pairs, there key is a translated (with a current language) name of language and value is its translation code name (pl, pt, de, ...).
 *
 * As the result is a QMap, it comes sorted by a translated names of languages.
 */
QMap<QString, QString> getAvailableLanguages();

#endif // TRANSLATIONS_H
