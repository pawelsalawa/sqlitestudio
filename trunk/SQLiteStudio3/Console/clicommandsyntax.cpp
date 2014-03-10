#include "clicommandsyntax.h"
#include <QDebug>

CliCommandSyntax::CliCommandSyntax()
{
}

CliCommandSyntax::~CliCommandSyntax()
{
    foreach (Argument* arg, arguments)
        delete arg;

    arguments.clear();
    argumentMap.clear();

    foreach (Option* opt, options)
        delete opt;

    optionMap.clear();
    optionsLongNameMap.clear();
    optionsShortNameMap.clear();
    options.clear();
}

void CliCommandSyntax::addArgument(int id, const QString& name, bool mandatory)
{
    addArgumentInternal(id, {name}, mandatory, Argument::REGULAR);
}

void CliCommandSyntax::addStrictArgument(int id, const QStringList& values, bool mandatory)
{
    addArgumentInternal(id, values, mandatory, Argument::STRICT);
}

void CliCommandSyntax::addAlternatedArgument(int id, const QStringList& names, bool mandatory)
{
    addArgumentInternal(id, names, mandatory, Argument::ALTERNATED);
}

void CliCommandSyntax::addOptionShort(int id, const QString& shortName)
{
    addOptionInternal(id, shortName, QString(), QString());
}

void CliCommandSyntax::addOptionLong(int id, const QString& longName)
{
    addOptionInternal(id, QString(), longName, QString());
}

void CliCommandSyntax::addOption(int id, const QString& shortName, const QString& longName)
{
    addOptionInternal(id, shortName, longName, QString());
}

void CliCommandSyntax::addOptionWithArgShort(int id, const QString& shortName, const QString& argName)
{
    addOptionInternal(id, shortName, QString(), argName);
}

void CliCommandSyntax::addOptionWithArgLong(int id, const QString& longName, const QString& argName)
{
    addOptionInternal(id, QString(), longName, argName);
}

void CliCommandSyntax::addOptionWithArg(int id, const QString& shortName, const QString& longName, const QString& argName)
{
    addOptionInternal(id, shortName, longName, argName);
}

CliCommandSyntax::Argument* CliCommandSyntax::addArgumentInternal(int id, const QStringList& names, bool mandatory, Argument::Type type)
{
    checkNewArgument(mandatory);

    Argument* arg = new Argument;
    arg->mandatory = mandatory;
    arg->id = id;
    arg->type = type;
    arg->names = names;
    arguments << arg;
    argumentMap[id] = arg;
    return arg;
}

CliCommandSyntax::Option* CliCommandSyntax::addOptionInternal(int id, const QString& shortName, const QString& longName, const QString& argName)
{
    Option* opt = new Option;
    opt->shortName = shortName;
    opt->longName = longName;
    opt->id = id;
    opt->argName = argName;
    optionMap[id] = opt;
    options << opt;
    optionsShortNameMap[shortName] = opt;
    optionsLongNameMap[longName] = opt;
    return opt;
}

bool CliCommandSyntax::getStrictArgumentCount() const
{
    return strictArgumentCount;
}

void CliCommandSyntax::setStrictArgumentCount(bool value)
{
    strictArgumentCount = value;
}


bool CliCommandSyntax::parse(const QStringList& args)
{
    bool pastOptions = false;
    QString arg;
    bool res = false;
    int argCnt = args.size();
    for (int argIdx = 0; argIdx < argCnt; argIdx++)
    {
        arg = args[argIdx];

        if (pastOptions)
        {
            res = parseArg(arg);
        }
        else if (arg == "--")
        {
            pastOptions = true;
            res = true;
        }
        else if (arg.startsWith("-"))
        {
            res = parseOpt(arg, args, argIdx);
        }
        else
        {
            pastOptions = true;
            res = parseArg(arg);
        }

        if (!res)
            return false;
    }

    if (strictArgumentCount && argPosition < requiredArguments())
    {
        parsingErrorText = QObject::tr("Insufficient number of arguments.");
        return false;
    }

    return true;
}

QString CliCommandSyntax::getErrorText() const
{
    return parsingErrorText;
}

void CliCommandSyntax::addAlias(const QString& alias)
{
    aliases << alias;
}

QStringList CliCommandSyntax::getAliases() const
{
    return aliases;
}

QString CliCommandSyntax::getSyntaxDefinition() const
{
    return getSyntaxDefinition(name);
}

QString CliCommandSyntax::getSyntaxDefinition(const QString& usedName) const
{
    static const QString mandatoryArgTempl = "<%1>";
    static const QString optionalArgTempl = "[<%1>]";
    static const QString optionTempl = "[%1]";
    static const QString optionWithArgTempl = "[%1 <%2>]";

    QStringList words;
    words << usedName;

    QString optName;
    QStringList optNameParts;
    foreach (Option* opt, options)
    {
        optNameParts.clear();;
        if (!opt->shortName.isEmpty())
            optNameParts << "-"+opt->shortName;

        if (!opt->longName.isEmpty())
            optNameParts += "--"+opt->longName;

        optName = optNameParts.join("|");

        words << (opt->argName.isEmpty() ? optionTempl.arg(optName) : optionWithArgTempl.arg(optName).arg(opt->argName));
    }

    QString argName;
    foreach (Argument* arg, arguments)
    {
        switch (arg->type)
        {
            case CliCommandSyntax::Argument::ALTERNATED:
                argName = arg->names.join("|");
                break;
            case CliCommandSyntax::Argument::REGULAR:
                argName = arg->names.first();
                break;
            case CliCommandSyntax::Argument::STRICT:
                argName = arg->names.join("|");
                break;
        }
        words << (arg->mandatory ? mandatoryArgTempl.arg(argName) : optionalArgTempl.arg(argName));
    }

    return words.join(" ");
}

bool CliCommandSyntax::isArgumentSet(int id) const
{
    if (!argumentMap.contains(id))
        return false;

    return argumentMap[id]->defined;
}

QString CliCommandSyntax::getArgument(int id) const
{
    if (!argumentMap.contains(id))
        return QString::null;

    return argumentMap[id]->value;
}

bool CliCommandSyntax::isOptionSet(int id) const
{
    if (!optionMap.contains(id))
        return false;

    return optionMap[id]->requested;
}

QString CliCommandSyntax::getOptionValue(int id) const
{
    if (!optionMap.contains(id))
        return QString::null;

    return optionMap[id]->value;
}

bool CliCommandSyntax::parseArg(const QString& arg)
{
    if (strictArgumentCount && arguments.size() < (argPosition + 1))
    {
        parsingErrorText = QObject::tr("Too many arguments.");
        return false;
    }

    switch (arguments[argPosition]->type)
    {
        case CliCommandSyntax::Argument::ALTERNATED:
        {
            arguments[argPosition]->value = arg;
            arguments[argPosition]->defined = true;
            break;
        }
        case CliCommandSyntax::Argument::REGULAR:
        {
            arguments[argPosition]->value = arg;
            arguments[argPosition]->defined = true;
            break;
        }
        case CliCommandSyntax::Argument::STRICT:
        {
            if (!arguments[argPosition]->names.contains(arg))
            {
                parsingErrorText = QObject::tr("Invalid argument value: %1.\nExpected one of: %2").arg(arg)
                        .arg(arguments[argPosition]->names.join(", "));

                return false;
            }
            arguments[argPosition]->value = arg;
            arguments[argPosition]->defined = true;
            break;
        }
        default:
            qCritical() << "Invalid argument type in CliCommandSyntax:" << arguments[argPosition]->type;
            return false;
    }

    argPosition++;
    return true;
}

bool CliCommandSyntax::parseOpt(const QString& arg, const QStringList& args, int& argIdx)
{
    Option* opt = nullptr;
    if (arg.startsWith("--"))
    {
        QString longName = arg.mid(2);
        if (optionsLongNameMap.contains(longName))
            opt = optionsLongNameMap.value(longName);
    }
    else
    {
        QString shortName = arg.mid(1, 1);
        if (optionsShortNameMap.contains(shortName))
            opt = optionsShortNameMap.value(shortName);
    }

    if (!opt)
    {
        parsingErrorText = QObject::tr("Unknown option: %1", "CLI command syntax").arg(arg);
        return false;
    }

    opt->requested = true;

    if (!opt->argName.isEmpty())
    {
        if (args.size() < (argIdx + 1))
        {
            parsingErrorText = QObject::tr("Option %1 requires an argument.", "CLI command syntax").arg(arg);
            return false;
        }

        argIdx++;
        opt->value = args[argIdx];
    }
    return true;
}

int CliCommandSyntax::requiredArguments() const
{
    int cnt = 0;
    foreach (Argument* arg, arguments)
    {
        if (arg->mandatory)
            cnt++;
    }
    return cnt;
}

void CliCommandSyntax::checkNewArgument(bool mandatory)
{
    if (arguments.size() > 0 && !arguments.last()->mandatory && mandatory)
    {
        qWarning() << "Adding mandatory CLI command argument after optional argument. This will result in invalid syntax definition. The command is:"
                   << this->name;
    }
}

QString CliCommandSyntax::getName() const
{
    return name;
}

void CliCommandSyntax::setName(const QString& value)
{
    name = value;
}

