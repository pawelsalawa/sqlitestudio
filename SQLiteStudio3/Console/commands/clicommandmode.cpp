#include "clicommandmode.h"
#include "unused.h"
#include "cli_config.h"

bool CliCommandMode::execute(QStringList args)
{
    if (args.size() == 0)
    {
        println(tr("Current results printing mode: %1").arg(CliResultsDisplay::mode(CFG_CLI.Console.ResultsDisplayMode.get())));
        return false;
    }

    CliResultsDisplay::Mode mode = CliResultsDisplay::mode(args[0].toUpper());
    if (args[0].toUpper() != CliResultsDisplay::mode(mode))
    {
        println(tr("Invalid results printing mode: %1").arg(args[0].toUpper()));
        return false;
    }

    CFG_CLI.Console.ResultsDisplayMode.set(mode);
    println(tr("New results printing mode: %1").arg(CliResultsDisplay::mode(mode)));

    return false;
}

bool CliCommandMode::validate(QStringList args)
{
    UNUSED(args);
    return true;
}

QString CliCommandMode::shortHelp() const
{
    return tr("tells or changes the query results format");
}

QString CliCommandMode::fullHelp() const
{
    return tr(
                "When called without arguments, tells the current output format for a query results. "
                "When the <mode> is passed, the mode is changed to the given one. "
                "Some of modes require additional arguments.\n"
                "Supported modes are:\n"
                "- CLASSIC <separator> - columns are separated by <separator>, not aligned,\n"
                "- FIXED               - columns have fixed width, they always fit into terminal window width, but the data in columns can be cut off,\n"
                "- ROW                 - each column from the row is displayed in new line, so the full data is displayed.\n"
                "\n"
                "The CLASSIC mode is recommended if you want to see all the data, but you don't want to waste lines for each column. "
                "Each row will display full data for every column, but this also means, that columns will not be aligned to each other in next rows. "
                "The CLASSIC mode also doesn't respect the width of your terminal (console) window, so if values in columns are wider than the window, "
                "the row will be continued in next lines.\n\n"
                "The FIXED mode is recommended if you want a readable output and you don't care about long data values. "
                "Columns will be aligned, making the output a nice table. The width of columns is calculated from width of the console window "
                "and a number of columns.\n\n"
                "The ROW mode is recommended if you need to see whole values and you don't expect many rows to be displayed, because this mode "
                "displays a line of output per each column, so you'll get 10 lines for single row with 10 columns, then if you have 10 of such rows, "
                "you will get 100 lines of output (+1 extra line per each row, to separate rows from each other)."
             );
}

QString CliCommandMode::usage() const
{
    return "mode "+tr("<mode> [<parameter> [<parameter> ...]]");
}
