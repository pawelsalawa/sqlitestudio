#include "clicommandhistory.h"
#include "cli.h"
#include "utils.h"
#include "cliutils.h"

void CliCommandHistory::execute()
{
    if (syntax.isArgumentSet(OPER_TYPE))
    {
        cli->clearHistory();
        println(tr("Console history erased."));
        return;
    }

    int cols = getCliColumns();
    QString hline = pad("", cols, '-');
    foreach (const QString& line, cli->getHistory())
    {
        print(hline);
        println(line);
    }
    println(hline);
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

void CliCommandHistory::defineSyntax()
{
    syntax.setName("history");
    syntax.addStrictArgument(OPER_TYPE, {"clear"}, false);
}
