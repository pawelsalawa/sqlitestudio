#ifndef CLICOMMANDFACTORY_H
#define CLICOMMANDFACTORY_H

#include <QString>
#include <QHash>

class CliCommand;

class CliCommandFactory
{
    public:
        static void init();
        static CliCommand* getCommand(const QString& cmdName);
        static QHash<QString,CliCommand*> getAllCommands();

    private:
        typedef CliCommand* (*CliCommandCreatorFunc)();

        static QHash<QString,CliCommandCreatorFunc> mapping;
};

#endif // CLICOMMANDFACTORY_H
