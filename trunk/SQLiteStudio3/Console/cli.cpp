#include "cli.h"
#include "sqlitestudio.h"
#include "config.h"
#include "cli_config.h"
#include "db/dbmanager.h"
#include "commands/clicommandfactory.h"
#include "commands/clicommand.h"
#include "qio.h"
#include "readline.h"
#include <QCoreApplication>
#include <QThread>
#include <QFile>
#include <QSet>
#include <QStringList>
#include <QLibrary>

CLI::CLI(QObject *parent) :
    QObject(parent)
{
    currentDb = nullptr;
    maxResultColumnLength = 50;
}

CLI::~CLI()
{
}

void CLI::start()
{
    dbManager =  SQLiteStudio::getInstance()->getDbManager();

    thread = new QThread(this);

    CliCommandFactory::init();

    connect(thread, &QThread::started, this, &CLI::doWork);
    connect(thread, &QThread::finished, this, &CLI::done);
    this->moveToThread(thread);

    Db* db = dbManager->getByName(CLI_CFG.General.DefaultDatabase.get());
    if (db)
        currentDb = db;
    else
    {
        QList<Db*> dbList = dbManager->getDbList();
        if (dbList.size() > 0)
            currentDb = dbList[0];
        else
            currentDb = nullptr;
    }

    if (currentDb)
        qOut << tr("Current database: %1").arg(currentDb->getName()) << "\n\n";
    else
        qOut << tr("No current working database is set.") << "\n\n";

    qOut.flush();

    thread->start();
}

void CLI::setCurrentDb(Db* db)
{
    currentDb = db;
}

Db* CLI::getCurrentDb()
{
    return currentDb;
}

quint32 CLI::getMaxColLength()
{
    return maxResultColumnLength;
}

void CLI::setMaxColLength(quint32 value)
{
    maxResultColumnLength = value;
}

void CLI::exit()
{
    doExit = true;
}

void CLI::println(const QString &msg)
{
    qOut << msg << "\n";
    qOut.flush();
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

void CLI::doWork()
{
    static const QString prompt = "%1>";

    CliCommand* cliCommand;
    QString cmd;
    QStringList cmdArgs;
    QString cPrompt;
    QString line;
    char *cline;
    while (!doExit)
    {
        line.clear();

        while (!doExit && line.isEmpty())
        {
            cPrompt = currentDb ? prompt.arg(currentDb->getName()) : "";
            cline = readline(cPrompt.toLatin1().data());
            line = cline;
            free(cline);

            line = line.trimmed();
        }
        add_history(line.toLatin1().data());

        if (line.startsWith("."))
        {

            cmdArgs = line.mid(1).split(QRegExp("\\s+"));
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
        if (!cliCommand->validate(cmdArgs))
        {
            delete cliCommand;
            continue;
        }

        cliCommand->moveToThread(qApp->thread());
        emit execCommand(cliCommand, cmdArgs);
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
