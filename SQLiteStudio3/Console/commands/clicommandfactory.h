#ifndef CLICOMMANDFACTORY_H
#define CLICOMMANDFACTORY_H

#include <QString>
#include <QHash>

class CliCommand;

typedef CliCommand* (*CliCommandCreatorFunc)();

class CliCommandFactory
{
    public:
        static void init();
        static CliCommand* getCommand(const QString& cmdName);
        static CliCommand* getSqlCommand();
        static void registerCommand(const QString& name, CliCommandCreatorFunc creator);

    private:
        static QHash<QString,CliCommandCreatorFunc> mapping;
};

#endif // CLICOMMANDFACTORY_H
