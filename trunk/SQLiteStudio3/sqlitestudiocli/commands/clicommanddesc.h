#ifndef CLICOMMANDDESC_H
#define CLICOMMANDDESC_H

#include "clicommand.h"

class SqliteCreateTable;
class SqliteCreateVirtualTable;

class CliCommandDesc : public CliCommand
{
        Q_OBJECT

    public:
        CliCommandDesc();
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        void printTable(SqliteCreateTable* table);
        void printVirtualTable(SqliteCreateVirtualTable* table);
        void printHorizontalLine(int lgt3rd);
};

#endif // CLICOMMANDDESC_H
