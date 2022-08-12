#include "codesnippetmanager.h"
#include "common/utils.h"
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
    QHash<QString,QVariant> snHash;
    for (CodeSnippet*& snip : allSnippets)
    {
        snHash["name"] = snip->name;
        snHash["code"] = snip->code;
        snHash["hoteky"] = snip->hotkey;
        list << snHash;
    }
    CFG_CORE.Internal.CodeSnippets.set(list);
}

QString CodeSnippetManager::getCodeByName(const QString& name) const
{
    CodeSnippet* snippet = findFirst<CodeSnippet>(allSnippets, [name](CodeSnippet* snippet) -> bool
    {
        return snippet->name == name;
    });
    return snippet ? snippet->code : QString();
}

void CodeSnippetManager::loadFromConfig()
{
    clearSnippets();

    QVariantList list = CFG_CORE.Internal.CodeSnippets.get();
    QHash<QString,QVariant> snHash;
    CodeSnippet* snip = nullptr;
    for (const QVariant& var : list)
    {
        snHash = var.toHash();
        snip = new CodeSnippet();
        snip->name = snHash["name"].toString();
        snip->code = snHash["code"].toString();
        snip->hotkey = snHash["hoteky"].toString();
        allSnippets << snip;
    }
    refreshNames();
}

void CodeSnippetManager::refreshNames()
{
    names = map<CodeSnippet*, QString>(allSnippets, [](CodeSnippet* snippet) -> QString
    {
        return snippet->name;
    });
}

void CodeSnippetManager::clearSnippets()
{
    for (CodeSnippet*& snip : allSnippets)
        delete snip;

    allSnippets.clear();
}

void CodeSnippetManager::createDefaultSnippets()
{
    CodeSnippet* snip = new CodeSnippet();
    snip->name = "Create Table";
    snip->code = "CREATE TABLE tableName (\n"
                  "   id      INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                  "   value   TEXT,\n"
                  "   image   BLOB\n"
                  ");";
    snip->hotkey = "c";
    allSnippets << snip;

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
    allSnippets << snip;

    saveToConfig();
    CFG_CORE.Internal.DefaultSnippetsCreated.set(true);
}
