#include "cli.h"
#include "sqlitestudio.h"
#include "config.h"
#include "cli_config.h"
#include "db/dbmanager.h"
#include "commands/clicommandfactory.h"
#include "commands/clicommand.h"
#include "qio.h"
#include "utils.h"
#include "utils_sql.h"
#include "climsghandler.h"
#include "clicompleter.h"
#include <QCoreApplication>
#include <QThread>
#include <QFile>
#include <QSet>
#include <QStringList>
#include <QLibrary>

#if defined(Q_OS_WIN32)
#include "readline.h"
#elif defined(Q_OS_UNIX)
#include <readline/readline.h>
#include <readline/history.h>
#endif

CLI* CLI::instance = nullptr;

CLI::CLI(QObject* parent) :
    QObject(parent)
{
    setCurrentDb(nullptr);

    using_history();

#ifdef Q_OS_UNIX
    history_base = 0; // for some reason this was set to 1 under Unix, making 1st history entry to be always ommited
#endif


    loadHistory();
    CliCompleter::getInstance()->init(this);
}

CLI::~CLI()
{
}

CLI* CLI::getInstance()
{
    if (!instance)
        instance = new CLI();

    return instance;
}

void CLI::start()
{
    thread = new QThread(this);

    CliCommandFactory::init();

    connect(thread, &QThread::started, this, &CLI::doWork);
    connect(thread, &QThread::finished, this, &CLI::done);
    this->moveToThread(thread);

    if (!getCurrentDb()) // it could be set by openDbFile() from main().
    {
        Db* db = DBLIST->getByName(CFG_CLI.Console.DefaultDatabase.get());
        if (db)
        {
            setCurrentDb(db);
        }
        else
        {
            QList<Db*> dbList = DBLIST->getDbList();
            if (dbList.size() > 0)
                setCurrentDb(dbList[0]);
            else
                setCurrentDb(nullptr);
        }
    }

    qOut << QString("\n%1 (%2)\n------------------------\n\n").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());
    qOut.flush();

    if (getCurrentDb())
        qOut << tr("Current database: %1").arg(getCurrentDb()->getName()) << "\n\n";
    else
        qOut << tr("No current working database is set.") << "\n\n";

    qOut.flush();

    thread->start();
}

void CLI::setCurrentDb(Db* db)
{
    currentDb = db;
    if (db && !db->isOpen())
        db->open();
}

Db* CLI::getCurrentDb() const
{
    return currentDb;
}

void CLI::exit()
{
    doExit = true;
}

QStringList CLI::getHistory() const
{
    QStringList cfgHistory;

    int length = historyLength();

    QString line;
    HIST_ENTRY* entry;
    for (int i = 0; i < length; i++)
    {
        entry = history_get(i);
        if (!entry)
        {
            qWarning() << "Null history entry for i =" << i;
            continue;
        }

        line = QString::fromLocal8Bit(entry->line);
        if (line.isEmpty())
            continue;

        cfgHistory << line;
    }
    return cfgHistory;
}

void CLI::println(const QString &msg)
{
    qOut << msg << "\n";
    qOut.flush();
}

int CLI::historyLength() const
{
#if defined(Q_OS_WIN)
    return history_length();
#elif defined(Q_OS_UNIX)
    return history_length;
#endif
}

void CLI::waitForExecution()
{
    executionFinished = false;
    while (!executionFinished)
    {
        qApp->processEvents();
        QThread::usleep(20);
    }
}

bool CLI::isComplete(const QString& contents) const
{
    if (contents.startsWith(CFG_CLI.Console.CommandPrefixChar.get()))
        return true;

    Dialect dialect = Dialect::Sqlite3;
    if (getCurrentDb())
        dialect = getCurrentDb()->getDialect();

    bool complete;
    splitQueries(contents, dialect, &complete);
    return complete;
}

void CLI::loadHistory()
{
    foreach (const QString& line, CFG->getCliHistory())
    {
        if (!line.isEmpty())
            add_history(line.toLocal8Bit().data());
    }
}

void CLI::addHistory(const QString& text)
{
    CFG->addCliHistory(text);

    add_history(text.toLocal8Bit().data());

    if (historyLength() > CFG_CORE.Console.HistorySize.get())
        free_history_entry(remove_history(0));
}

QString CLI::getLine() const
{
    return line;
}

void CLI::applyHistoryLimit()
{
    CFG->applyCliHistoryLimit();
    while (historyLength() > CFG_CORE.Console.HistorySize.get())
        free_history_entry(remove_history(0));
}

void CLI::openDbFile(const QString& path)
{
    QString newName = DbManager::generateDbName(path);
    if (!DBLIST->addDb(newName, path, false))
    {
        println(tr("Could not add database %1 to list.").arg(path));
        return;
    }
    Db* db = DBLIST->getByName(newName);
    setCurrentDb(db);
}

void CLI::doWork()
{
    static const QString prompt = "%1>";

    CliCommand* cliCommand;
    QString cmd;
    QStringList cmdArgs;
    QString cPrompt;
    char *cline;
    while (!doExit)
    {
        line.clear();

        while (!doExit && (line.isEmpty() || !isComplete(line)))
        {
            if (getCurrentDb())
            {
                cPrompt = getCurrentDb()->getName();
                if (!getCurrentDb()->isOpen())
                    cPrompt += " ["+tr("closed")+"]";

                cPrompt = prompt.arg(cPrompt);
            }
            else
                cPrompt = prompt.arg("");

            if (!line.isEmpty())
            {
                cPrompt = pad("->", -cPrompt.length(), ' ');
                line += "\n";
            }

            cline = readline(cPrompt.toLocal8Bit().data());

            line += cline;
            free(cline);
        }
        addHistory(line);

        if (line.startsWith(CFG_CLI.Console.CommandPrefixChar.get()))
        {

            cmdArgs = tokenizeArgs(line.mid(1));
            cmd = cmdArgs.takeAt(0);
            cliCommand = CliCommandFactory::getCommand(cmd);
            if (!cliCommand)
            {
                println("No such command.");
                continue;
            }
        }
        else
        {
            cliCommand = CliCommandFactory::getCommand("query");
            cmdArgs.clear();
            cmdArgs << line;
        }

        cliCommand->setup(this);
        if (!cliCommand->parseArgs(cmdArgs))
        {
            delete cliCommand;
            continue;
        }

        cliCommand->moveToThread(qApp->thread());
        emit execCommand(cliCommand);
        waitForExecution();
    }

    thread->quit();
}

void CLI::done()
{
    qApp->exit();
}

void CLI::executionComplete()
{
    executionFinished = true;
}

void CLI::clearHistory()
{
    clear_history();
    CFG->clearCliHistory();
}
