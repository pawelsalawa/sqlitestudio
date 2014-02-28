#ifndef CLICOMMANDSQL_H
#define CLICOMMANDSQL_H

#include "clicommand.h"

class CliCommandSql : public CliCommand
{
        Q_OBJECT

    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;

    private slots:
        void executionFailed(int code, const QString& msg);
};

#endif // CLICOMMANDSQL_H
