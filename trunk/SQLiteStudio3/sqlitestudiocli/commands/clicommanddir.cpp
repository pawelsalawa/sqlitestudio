#include "clicommanddir.h"
#include <QDir>

void CliCommandDir::execute()
{
    QDir dir;
    QFileInfoList entries;

    if (syntax.isArgumentSet(DIR_OR_FILE))
    {
        QString filter = getFilterAndFixDir(dir, syntax.getArgument(DIR_OR_FILE));
        entries = dir.entryInfoList({filter}, QDir::AllEntries|QDir::NoDotAndDotDot, QDir::DirsFirst|QDir::LocaleAware|QDir::Name);
    }
    else
        entries = dir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot, QDir::DirsFirst|QDir::LocaleAware|QDir::Name);

    QString name;
    foreach (const QFileInfo& entry, entries)
    {
        name = entry.fileName();
        if (entry.isDir())
            name += "/";

        if (dir != QDir::current())
            name.prepend(dir.path()+"/");

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
                "You can pass <pattern> with wildcard characters to filter output."
             );
}

void CliCommandDir::defineSyntax()
{
    syntax.setName("dir");
    syntax.addAlias("ls");
    syntax.addArgument(DIR_OR_FILE, tr("pattern"), false);
}
