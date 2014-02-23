#include "cli.h"
#include "sqlitestudio.h"
#include "config.h"
#include "cli_config.h"
#include "db/dbmanager.h"
#include "commands/clicommandfactory.h"
#include "commands/clicommand.h"
#include "qio.h"
#include "lineedit.h"
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
    waitMutex.lock();
    executionFinished = false;
    while (!executionFinished)
    {
        qApp->processEvents();
        waitForExec.wait(&waitMutex, 20);
    }
    waitMutex.unlock();
}

void CLI::printPrompt()
{
    qOut << prompt;
    qOut.flush();
}

void CLI::doWork()
{
        prompt = "SQLiteStudio> ";

        DESC* fd = lineeditInit(NULL);

        CliCommand* cliCommand;
        QString cmd;
        QStringList cmdArgs;
        QString line;
        char *cline;
        while (!doExit)
        {
            line.clear();
            printPrompt();

            while (!doExit && line.isEmpty())
            {
                qApp->processEvents();
                thread->usleep(20000);

                cline = lineedit(fd, prompt.toLatin1().data());
                line = cline;
                free(cline);

                line = line.trimmed();
            }

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
                cliCommand = CliCommandFactory::getSqlCommand();
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
    waitForExec.wakeAll();
}
