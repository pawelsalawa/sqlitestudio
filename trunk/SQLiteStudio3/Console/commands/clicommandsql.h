#ifndef CLICOMMANDSQL_H
#define CLICOMMANDSQL_H

#include "clicommand.h"
#include "db/sqlresults.h"

class QueryExecutor;

class CliCommandSql : public CliCommand
{
        Q_OBJECT

    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;

    private:
        void printResultsClassic(QueryExecutor *executor, SqlResultsPtr results);
        void printResultsFixed(QueryExecutor *executor, SqlResultsPtr results);
        void printResultsRowByRow(QueryExecutor *executor, SqlResultsPtr results);

        QString getValueString(const QVariant& value);

    private slots:
        void executionFailed(int code, const QString& msg);
};

#endif // CLICOMMANDSQL_H
