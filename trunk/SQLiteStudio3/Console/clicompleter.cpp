#include "clicompleter.h"
#include "completionhelper.h"
#include "cli.h"
#include "cli_config.h"
#include "unused.h"
#include "commands/clicommand.h"
#include "parser/lexer.h"
#include <QString>
#include <QDebug>

#if defined(Q_OS_WIN32)
#include "readline.h"
#include <commands/clicommandfactory.h>
#elif defined(Q_OS_UNIX)
#include <readline/readline.h>
#endif

CliCompleter* CliCompleter::instance = nullptr;

CliCompleter* CliCompleter::getInstance()
{
    if (!instance)
        instance = new CliCompleter();

    return instance;
}

char** CliCompleter::complete(const char* text, int start, int end)
{
    UNUSED(start);
    return toCharArray(getInstance()->completeInternal(QString::fromLocal8Bit(text), QString::fromLocal8Bit(rl_line_buffer), end));
}

QStringList CliCompleter::completeInternal(const QString& toBeReplaced, const QString& text, int curPos)
{
    QString str;
    if (!cli->getLine().isEmpty())
    {
        str += cli->getLine();
        curPos += str.length();
    }

    str += text;

    if (str.startsWith(CFG_CLI.Console.CommandPrefixChar.get()))
        return completeCommand(str, curPos);

    bool keepOriginalStr = doKeepOriginalStr(str, curPos);

    CompletionHelper::Results results = CompletionHelper::getExpectedTokens(str, curPos, cli->getCurrentDb());
    QList<ExpectedTokenPtr> expectedTokens = results.filtered();

    QStringList list;
    foreach (const ExpectedTokenPtr& token, expectedTokens)
        list << token->value;

    list.removeAll("");
    if (list.size() > 1)
        list.removeOne(";"); // we don't want it together with other proposals, cause it introduces problems when proposed by completer

    list.removeDuplicates();

    if (keepOriginalStr)
    {
        QMutableStringListIterator it(list);
        while (it.hasNext())
            it.next().prepend(toBeReplaced);
    }

    if (list.size() == 1)
        list[0] += " ";

    return list;
}

QStringList CliCompleter::completeCommand(const QString& str, int curPos)
{
    QStringList results;
    QString text = str.mid(0, curPos);

    QStringList cmdWords = tokenizeArgs(text);
    if (cmdWords.size() == 0)
        return results;

    if (text[text.length()-1].isSpace())
        cmdWords << "";

    QString cmdStr = cmdWords[0].mid(1);
    if (cmdWords.size() > 1)
    {
        CliCommand* command = CliCommandFactory::getCommand(cmdStr);
        if (!command)
            return results;

        command->setup(cli);
        results = command->complete(cmdWords.mid(1)).filter(QRegExp("^"+cmdWords.last()+".*"));
    }
    else
    {
        QStringList cmdNames = CliCommandFactory::getCommandNames().filter(QRegExp("^"+cmdStr+".*"));
        cmdNames.sort(Qt::CaseInsensitive);
        foreach (const QString& cmdName, cmdNames)
            results << CFG_CLI.Console.CommandPrefixChar.get() + cmdName;
    }

    results.removeDuplicates();

    if (results.size() == 1)
        results[0] += " ";

    return results;
}

bool CliCompleter::doKeepOriginalStr(const QString& str, int curPos)
{
    Dialect dialect = Dialect::Sqlite3;
    if (cli->getCurrentDb())
        dialect = cli->getCurrentDb()->getDialect();

    TokenList tokens = Lexer::tokenize(str.mid(0, curPos), dialect);
    if (tokens.size() == 0)
        return false;

    switch (tokens.last()->type)
    {
        case Token::SPACE:
        case Token::PAR_LEFT:
        case Token::PAR_RIGHT:
        case Token::OPERATOR:
            return true;
        default:
            break;
    }
    return false;
}

char** CliCompleter::toCharArray(const QStringList& list)
{
    char** array = (char**)malloc((list.size() + 1) * sizeof(char*));
    array[list.size()] = nullptr;

    int i = 0;
    foreach (const QString& str, list)
        array[i++] = _strdup(str.toLocal8Bit().data());

    return array;
}

CliCompleter::CliCompleter()
{
}

void CliCompleter::init(CLI* value)
{
    rl_attempted_completion_function = CliCompleter::complete;
    cli = value;
}

