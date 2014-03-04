#include "clicommandhistory.h"
#include "cli.h"
#include "utils.h"
#include "cliutils.h"

bool CliCommandHistory::execute(QStringList args)
{
    if (args.size() == 0)
    {
        int cols = getCliColumns();
        QString hline = pad("", cols-1, '-');
        foreach (const QString& line, cli->getHistory())
        {
            println(hline);
            println(line);
        }
        println(hline);
        return false;
    }

    cli->clearHistory();
    println(tr("Console history erased."));
    return false;
}

bool CliCommandHistory::validate(QStringList args)
{
    if (args.size() > 1)
    {
        printUsage();
        return false;
    }

    if (args.size() == 1 && args[0] != "clear")
    {
        printUsage();
        return false;
    }

    return true;
}

QString CliCommandHistory::shortHelp() const
{
    return tr("prints history or erases it");
}

QString CliCommandHistory::fullHelp() const
{
    return tr(
                "When no argument was passed, this command prints command line history. "
                "Every history entry is separated with a horizontal line, so multiline entries are easier to read.\n"
                "\n"
                "When the 'clear' argument is passed, then the history gets erased."
             );
}

QString CliCommandHistory::usage() const
{
    return "history "+tr("[clear]");
}
