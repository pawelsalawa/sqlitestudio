#include "clicommanddesc.h"

CliCommandDesc::CliCommandDesc()
{
}

void CliCommandDesc::execute()
{

}

QString CliCommandDesc::shortHelp() const
{
    return tr("shows details about the table");
}

QString CliCommandDesc::fullHelp() const
{
    return QString();
}

void CliCommandDesc::defineSyntax()
{
    syntax.setName("desc");
    syntax.addArgument(TABLE, tr("table"));
}
