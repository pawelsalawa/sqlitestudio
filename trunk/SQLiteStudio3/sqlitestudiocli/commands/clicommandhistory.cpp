#include "clicommandhistory.h"
#include "cli.h"
#include "common/utils.h"
#include "cliutils.h"
#include "services/config.h"

void CliCommandHistory::execute()
{
    if (syntax.isOptionSet(OPER_TYPE))
    {
        clear();
        return;
    }

    if (syntax.isOptionSet(HIST_LIMIT))
    {
        setMax(syntax.getOptionValue(HIST_LIMIT));
        return;
    }

    if (syntax.isOptionSet(SHOW_LIMIT))
    {
        println(tr("Current history limit is set to: %1").arg(CFG_CORE.Console.HistorySize.get()));
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
                "When the -c or --clear option is passed, then the history gets erased.\n"
                "When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument "
                "saying how many entries do you want the history to be limited to.\n"
                "Use -ql or --querylimit option to see the current limit value."
             );
}

void CliCommandHistory::defineSyntax()
{
    syntax.setName("history");
    syntax.addOption(OPER_TYPE, "c", "clear");
    syntax.addOptionWithArg(HIST_LIMIT, "l", "limit", tr("number"));
    syntax.addOption(SHOW_LIMIT, "ql", "querylimit");
}

void CliCommandHistory::clear()
{
    cli->clearHistory();
    println(tr("Console history erased."));
}

void CliCommandHistory::setMax(const QString& arg)
{
    bool ok;
    int max = arg.toInt(&ok);
    if (!ok)
    {
        println(tr("Invalid number: %1").arg(arg));
        return;
    }
    CFG_CORE.Console.HistorySize.set(max);
    cli->applyHistoryLimit();
    println(tr("History limit set to %1").arg(max));
}
