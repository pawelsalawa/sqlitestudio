#include "clicommand.h"
#include "sqlitestudio.h"
#include "qio.h"
#include <QFile>

CliCommand::CliCommand()
{

}

CliCommand::~CliCommand()
{
}

void CliCommand::setup(CLI *cli)
{
    this->cli = cli;
    this->config = SQLiteStudio::getInstance()->getConfig();
    this->dbManager = SQLiteStudio::getInstance()->getDbManager();
}

void CliCommand::println(const QString &str)
{
    qOut << str << "\n";
    qOut.flush();
}
