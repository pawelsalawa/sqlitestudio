#include "clicommanddir.h"
#include <QDir>

void CliCommandDir::execute(const QStringList& args)
{
    QDir dir;
    QFileInfoList entries;

    if (args.size() == 1)
        entries = dir.entryInfoList({args[0]}, QDir::AllEntries|QDir::NoDotAndDotDot, QDir::DirsFirst|QDir::LocaleAware|QDir::Name);
    else
        entries = dir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot, QDir::DirsFirst|QDir::LocaleAware|QDir::Name);

    QString name;
    foreach (const QFileInfo& entry, entries)
    {
        name = entry.fileName();
        if (entry.isDir())
            name += "/";

        println(name);
    }
}

QString CliCommandDir::shortHelp() const
{
    return tr("lists directories and files in current working directory");
}

QString CliCommandDir::fullHelp() const
{
    return tr(
                "This is very similar to 'dir' command known from Windows and 'ls' command from Unix systems.\n"
                "\n"
                "You can pass <pattern> with wildcard characters to filter output.\n"
                );
}

void CliCommandDir::defineSyntax()
{
    syntax.setName("dir");
    syntax.addAlias("ls");
    syntax.addArgument(PATTERN, tr("pattern"), false);
}
