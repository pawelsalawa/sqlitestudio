#include "clicompleter.h"
#include "completionhelper.h"
#include "cli.h"
#include "cli_config.h"
#include "common/unused.h"
#include "commands/clicommand.h"
#include "parser/lexer.h"
#include "commands/clicommandfactory.h"
#include <QString>
#include <QDebug>

#if defined(Q_OS_WIN32)
#include "readline.h"
#elif defined(Q_OS_UNIX)
#include <readline/readline.h>
#endif

CliCompleter* CliCompleter::instance = nullptr;

CliCompleter::CliCompleter()
{
}

void CliCompleter::init(CLI* value)
{
    rl_attempted_completion_function = CliCompleter::complete;
    cli = value;
}

CliCompleter* CliCompleter::getInstance()
{
    if (!instance)
        instance = new CliCompleter();

    return instance;
}

char** CliCompleter::complete(const char* text, int start, int end)
{
    UNUSED(start);

#ifdef Q_OS_UNIX
    // Unix readline needs this to disable the completion using rl_completion_entry_function
    rl_attempted_completion_over = 1;
#endif

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

    QStringList list;
    if (str.startsWith(CFG_CLI.Console.CommandPrefixChar.get()))
        list = completeCommand(str, curPos);
    else
        list = completeQuery(toBeReplaced, str, curPos);

    list.removeDuplicates();

#ifdef Q_OS_WIN
    if (list.size() == 1)
        list[0] += " ";
#endif

#ifdef Q_OS_UNIX
    // Unix readline treats first element in the list as a common value of all elements
    if (list.size() > 0)
        list.prepend(longestCommonPart(list));
#endif

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
        results = command->complete(cmdWords.mid(1)).filter(QRegularExpression("^"+cmdWords.last()+".*"));
    }
    else
    {
        QStringList cmdNames = CliCommandFactory::getCommandNames().filter(QRegularExpression("^"+cmdStr+".*"));
        cmdNames.sort(Qt::CaseInsensitive);
        for (const QString& cmdName : cmdNames)
            results << CFG_CLI.Console.CommandPrefixChar.get() + cmdName;
    }

    return results;
}

QStringList CliCompleter::completeQuery(const QString& toBeReplaced, const QString& str, int curPos)
{
    QStringList list;
    if (!cli->getCurrentDb())
        return list;

    bool keepOriginalStr = doKeepOriginalStr(str, curPos);

    CompletionHelper completer(str, curPos, cli->getCurrentDb());
    QList<ExpectedTokenPtr> expectedTokens = completer.getExpectedTokens().filtered();

    for (const ExpectedTokenPtr& token : expectedTokens)
        list << token->value;

    list.removeAll("");
    if (list.size() > 1)
        list.removeOne(";"); // we don't want it together with other proposals, cause it introduces problems when proposed by completer

    if (keepOriginalStr)
    {
        QMutableStringListIterator it(list);
        while (it.hasNext())
            it.next().prepend(toBeReplaced);
    }
    return list;
}

bool CliCompleter::doKeepOriginalStr(const QString& str, int curPos)
{
    TokenList tokens = Lexer::tokenize(str.mid(0, curPos));
    if (tokens.size() == 0)
        return false;

    return tokens.last()->isSeparating();
}

char** CliCompleter::toCharArray(const QStringList& list)
{
    if (list.size() == 0)
        return nullptr;

    char** array = (char**)malloc((list.size() + 1) * sizeof(char*));
    array[list.size()] = nullptr;

    int i = 0;
    for (const QString& str : list)
#if defined(Q_OS_WIN)
        array[i++] = _strdup(str.toLocal8Bit().data());
#elif defined(Q_OS_UNIX)
        array[i++] = strdup(str.toLocal8Bit().data());
#endif

    return array;
}
