#ifndef CLICOMMANDSQL_H
#define CLICOMMANDSQL_H

#include "clicommand.h"
#include "db/sqlresults.h"

class QueryExecutor;

class CliCommandSql : public CliCommand
{
        Q_OBJECT

    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        bool isAsyncExecution() const;
        void defineSyntax();

    private:
        void printResultsClassic(QueryExecutor *executor, SqlResultsPtr results);
        void printResultsFixed(QueryExecutor *executor, SqlResultsPtr results);
        void printResultsRowByRow(QueryExecutor *executor, SqlResultsPtr results);

        QString getValueString(const QVariant& value);

    private slots:
        void executionFailed(int code, const QString& msg);
};

#endif // CLICOMMANDSQL_H
