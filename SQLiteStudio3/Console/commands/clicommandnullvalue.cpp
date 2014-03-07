#include "clicommandnullvalue.h"
#include "cli_config.h"

void CliCommandNullValue::execute(const QStringList& args)
{
    if (args.size() == 1)
        CFG_CLI.Console.NullValue.set(args[0]);

    println(tr("Current NULL representation string: %1").arg(CFG_CLI.Console.NullValue.get()));
    return;
}

QString CliCommandNullValue::shortHelp() const
{
    return tr("tells or changes the NULL representation string");
}

QString CliCommandNullValue::fullHelp() const
{
    return tr(
                "If no argument was passed, it tells what's the current NULL value representation "
                "(that is - what is printed in place of NULL values in query results). "
                "If the argument is given, then it's used as a new string to be used for NULL representation."
                );
}

void CliCommandNullValue::defineSyntax()
{
    syntax.setName("null");
    syntax.addAlias("nullvalue");
    syntax.addArgument(STRING, QObject::tr("string", "CLI command syntax"), false);
}
