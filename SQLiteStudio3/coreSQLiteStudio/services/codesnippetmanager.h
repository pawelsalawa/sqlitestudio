#ifndef CODESNIPPETMANAGER_H
#define CODESNIPPETMANAGER_H

#include "coreSQLiteStudio_global.h"
#include <QObject>

class Config;

class API_EXPORT CodeSnippetManager : public QObject
{
    Q_OBJECT

    public:
        struct API_EXPORT CodeSnippet
        {
            QString name;
            QString code;
            QString hotkey;
        };

        CodeSnippetManager(Config* config);

        void setSnippets(const QList<CodeSnippet*>& snippets);
        const QList<CodeSnippet*>& getSnippets() const;
        const QStringList& getNames() const;
        void saveToConfig();
        QString getCodeByName(const QString& name) const;

    private:
        void loadFromConfig();
        void refreshNames();
        void clearSnippets();
        void createDefaultSnippets();

        Config* config = nullptr;
        QList<CodeSnippet*> allSnippets;
        QStringList names;
};

#define CODESNIPPETS SQLITESTUDIO->getCodeSnippetManager()

#endif // CODESNIPPETMANAGER_H
