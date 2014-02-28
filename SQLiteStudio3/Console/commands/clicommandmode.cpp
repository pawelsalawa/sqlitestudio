#include "clicommandmode.h"

bool CliCommandMode::execute(QStringList args)
{

}

bool CliCommandMode::validate(QStringList args)
{

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
                "Supported modes are:\n%1\n"
             ).arg(getModesHelp());
}

QString CliCommandMode::usage() const
{
    return "mode "+tr("<mode> [<parameter> [<parameter> ...]]");
}

QStringList CliCommandMode::getModes() const
{

}

QString CliCommandMode::getModesHelp() const
{

}
