#include "clicommandnullvalue.h"
#include "cli_config.h"

bool CliCommandNullValue::execute(QStringList args)
{
    if (args.size() == 1)
        CFG_CLI.Console.NullValue.set(args[0]);

    println(tr("Current NULL representation string: %1").arg(CFG_CLI.Console.NullValue.get()));
    return false;
}

bool CliCommandNullValue::validate(QStringList args)
{
    if (args.size() > 1)
    {
        printUsage();
        return false;
    }

    return true;
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

QString CliCommandNullValue::usage() const
{
    return "null "+tr("[<string>]");
}

QStringList CliCommandNullValue::aliases() const
{
    return {"nullvalue"};
}
