#include "codesnippetmanager.h"
#include "common/collections.h"
#include "services/config.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

CodeSnippetManager::CodeSnippetManager(Config* config) :
    config(config)
{
    loadFromConfig();
    if (!CFG_CORE.Internal.DefaultSnippetsCreated.get())
        createDefaultSnippets();
}

void CodeSnippetManager::setSnippets(const QList<CodeSnippet*>& snippets)
{
    clearSnippets();
    allSnippets = snippets;
    refreshNames();
    saveToConfig();
    emit codeSnippetListChanged();
}

void CodeSnippetManager::addSnippet(CodeSnippet* snippet)
{
    allSnippets << snippet;
    refreshNames();
    saveToConfig();
    emit codeSnippetListChanged();
}

const QList<CodeSnippetManager::CodeSnippet*>& CodeSnippetManager::getSnippets() const
{
    return allSnippets;
}

const QStringList& CodeSnippetManager::getNames() const
{
    return names;
}

void CodeSnippetManager::saveToConfig()
{
    QVariantList list;
    for (CodeSnippet*& snip : allSnippets)
        list << snip->toHash();

    CFG_CORE.Internal.CodeSnippets.set(list);
}

QString CodeSnippetManager::getCodeByName(const QString& name) const
{
    CodeSnippet* snippet = allSnippets | FIND_FIRST(snippet, {return snippet->name == name;});
    return snippet ? snippet->code : QString();
}

void CodeSnippetManager::loadFromConfig()
{
    clearSnippets();

    QVariantList list = CFG_CORE.Internal.CodeSnippets.get();
    CodeSnippet* snip = nullptr;
    for (const QVariant& var : list)
    {
        QHash<QString,QVariant> snHash = var.toHash();
        snip = new CodeSnippet();
        snip->name = snHash["name"].toString();
        snip->code = snHash["code"].toString();
        snip->hotkey = snHash["hoteky"].toString();
        allSnippets << snip;
    }
    refreshNames();
    emit codeSnippetListChanged();
}

void CodeSnippetManager::refreshNames()
{
    names = allSnippets | MAP(snippet, {return snippet->name;});
}

void CodeSnippetManager::clearSnippets()
{
    qDeleteAll(allSnippets);
    allSnippets.clear();
}

void CodeSnippetManager::createDefaultSnippets()
{
    allSnippets += createDefaultSnippetsFor(0);

    saveToConfig();
    CFG_CORE.Internal.DefaultSnippetsCreated.set(true);
}

QList<CodeSnippetManager::CodeSnippet*> CodeSnippetManager::createDefaultSnippetsFor(int appVersion)
{
    QList<CodeSnippetManager::CodeSnippet*> results;
    CodeSnippet* snip;

    // 0 is always for initial run when user had no configuration yet
    // Then 40000 is for version 4.0.0, in which basic snippets were added, so if user had config from 4.0.0 or older, they get those snippets as default ones.
    // Future versions can be added here with more default snippets, if needed, and users will get them if they update from version older than that.

    if (appVersion == 0 || appVersion == 40000)
    {
        snip = new CodeSnippet();
        snip->name = "Select From Table";
        snip->code = "SELECT * FROM tableName;";
        snip->hotkey = "s";
        results << snip;

        snip = new CodeSnippet();
        snip->name = "Select From Table, Where...";
        snip->code = "SELECT * FROM tableName WHERE conditions;";
        snip->hotkey = "Shift+S";
        results << snip;

        snip = new CodeSnippet();
        snip->name = "Delete From Table, Where...";
        snip->code = "DELETE FROM tableName WHERE conditions;";
        snip->hotkey = "d";
        results << snip;

        snip = new CodeSnippet();
        snip->name = "Insert Into Table";
        snip->code = "INSERT INTO tableName (columns) VALUES (values);";
        snip->hotkey = "i";
        results << snip;

        snip = new CodeSnippet();
        snip->name = "Update Table, Where...";
        snip->code = "UPDATE tableName SET assignments WHERE conditions;";
        snip->hotkey = "u";
        results << snip;
    }

    if (appVersion == 0)
    {
        snip = new CodeSnippet();
        snip->name = "Create Table";
        snip->code = "CREATE TABLE tableName (\n"
                     "   id      INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                     "   value   TEXT,\n"
                     "   image   BLOB\n"
                     ");";
        snip->hotkey = "c";
        results << snip;

        snip = new CodeSnippet();
        snip->name = "With Recursive";
        snip->code = "WITH RECURSIVE\n"
                      "   cnt(x) AS (\n"
                      "       SELECT 1\n"
                      "        UNION ALL\n"
                      "       SELECT x+1 FROM cnt\n"
                      "        LIMIT 1000000\n"
                      "   )\n"
                      "SELECT x FROM cnt;";
        snip->hotkey = "w";
        results << snip;
    }

    return results;
}

QVariantHash CodeSnippetManager::CodeSnippet::toHash() const
{
    return QVariantHash {
        {"name", name},
        {"code", code},
        {"hoteky", hotkey}
    };
}
