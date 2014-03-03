#include "clicommandsql.h"
#include "cli.h"
#include "parser/ast/sqliteselect.h"
#include "parser/parser.h"
#include "parser/parsererror.h"
#include "db/queryexecutor.h"
#include "qio.h"
#include "unused.h"
#include "cli_config.h"
#include "cliutils.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlField>
#include <QList>

bool CliCommandSql::execute(QStringList args)
{
    Db* db = cli->getCurrentDb();
    if (!db || !db->isOpen())
    {
        println(tr("Database is not open."));
        return false;
    }

    // Executor deletes itself later when called with lambda.
    QueryExecutor *executor = new QueryExecutor(db, args[0]);
    connect(executor, SIGNAL(executionFinished(SqlResultsPtr)), this, SIGNAL(execComplete()));
    connect(executor, SIGNAL(executionFailed(int,QString)), this, SLOT(executionFailed(int,QString)));
    connect(executor, SIGNAL(executionFailed(int,QString)), this, SIGNAL(execComplete()));

    executor->exec([=](SqlResultsPtr results)
    {
        if (results->isError())
            return; // should not happen, since results handler function is called only for successful executions

        switch (CFG_CLI.Console.ResultsDisplayMode.get())
        {
            case CliResultsDisplay::FIXED:
                printResultsFixed(executor, results);
                break;
            case CliResultsDisplay::ROW:
                printResultsRowByRow(executor, results);
                break;
            default:
                printResultsClassic(executor, results);
                break;
        }
    });

    return true;
}

bool CliCommandSql::validate(QStringList args)
{
    if (args.size() != 1)
    {
        printUsage();
        return false;
    }

    if (!cli->getCurrentDb())
    {
        println(tr("No working database is set.\n"
                   "Call %1 command to set working database.\n"
                   "Call %2 to see list of all databases.")
                .arg(cmdName("use")).arg(cmdName("dblist")));

        return false;
    }

    return true;
}

QString CliCommandSql::shortHelp() const
{
    return tr("executes SQL query");
}

QString CliCommandSql::fullHelp() const
{
    return tr(
                "This command is executed every time you enter SQL query in command prompt. "
                "It executes the query on the current working database (see help for %1 for details). "
                "There's no sense in executing this command explicitly. Instead just type the SQL query "
                "in the command prompt, without any command prefixed."
             ).arg(cmdName("use"));
}

QString CliCommandSql::usage() const
{
    return "query "+tr("<sql>");
}

void CliCommandSql::printResultsClassic(QueryExecutor* executor, SqlResultsPtr results)
{
    quint32 maxLength = CFG_CLI.Console.ColumnMaxWidth.get();
    int rowIdColumns = executor->getRowIdResultColumns().size();

    // Columns
    foreach (const QueryExecutor::ResultColumnPtr& resCol, executor->getResultColumns())
        qOut << resCol->displayName.left(maxLength) << "|";

    qOut << "\n";

    // Data
    SqlResultsRowPtr row;
    while (!(row = results->next()).isNull())
    {
        foreach (QVariant value, row->valueList().mid(rowIdColumns))
            qOut << value.toString().left(maxLength) << "|";

        qOut << "\n";
    }
    qOut.flush();
}

void CliCommandSql::printResultsFixed(QueryExecutor* executor, SqlResultsPtr results)
{
    QList<QueryExecutor::ResultColumnPtr> resultColumns = executor->getResultColumns();
    int resultColumnsCount = resultColumns.size();
    int rowIdColumns = executor->getRowIdResultColumns().size();
    int termCols = getCliColumns();
    int colWidth = termCols / resultColumns.size() - 1; // -1 for the separator

    if (colWidth == 0)
    {
        qOut << tr("Too many columns in results to display in the FIXED mode. "
                   "Either execute query with less columns in results, or enlarge console window, "
                   "or switch to different results display mode (see %1 command).").arg(cmdName("mode")) << "\n";
        return;
    }

    int i;
    int* widths = new int[resultColumnsCount];
    for (i = 0; i < resultColumnsCount; i++)
    {
        widths[i] = colWidth;
        if (i+1 == resultColumnsCount)
            widths[i] += (termCols - resultColumnsCount * (colWidth + 1));
    }

    // Columns
    QStringList line;
    i = 0;
    foreach (const QueryExecutor::ResultColumnPtr& resCol, executor->getResultColumns())
    {
        line << pad(resCol->displayName.left(widths[i]), widths[i], ' ');
        i++;
    }

    qOut << line.join("|") << "\n";

    line.clear();
    QString hline("-");
    for (i = 0; i < resultColumnsCount; i++)
    {
        line << hline.repeated(widths[i]);
    }

    qOut << line.join("+") << "\n";

    // Data
    SqlResultsRowPtr row;
    while (!(row = results->next()).isNull())
    {
        i = 0;
        line.clear();
        foreach (QVariant value, row->valueList().mid(rowIdColumns))
        {
            line << pad(value.toString().left(widths[i]), widths[i], ' ');
            i++;
        }

        qOut << line.join("|") << "\n";
    }
    qOut.flush();

    delete[] widths;
}

void CliCommandSql::printResultsRowByRow(QueryExecutor* executor, SqlResultsPtr results)
{
    // Columns
    int rowIdColumns = executor->getRowIdResultColumns().size();
    int colWidth = 0;
    foreach (const QueryExecutor::ResultColumnPtr& resCol, executor->getResultColumns())
    {
        if (resCol->displayName.length() > colWidth)
            colWidth = resCol->displayName.length();
    }

    QStringList columns;
    foreach (const QueryExecutor::ResultColumnPtr& resCol, executor->getResultColumns())
        columns << pad(resCol->displayName, -colWidth, ' ');

    // Data
    static const QString rowCntTemplate = tr("Row %1");
    int termWidth = getCliColumns();
    QString rowCntString;
    int i;
    int rowCnt = 1;
    SqlResultsRowPtr row;
    while (!(row = results->next()).isNull())
    {
        i = 0;
        rowCntString = " " + rowCntTemplate.arg(rowCnt) + " ";
        qOut << center(rowCntString, termWidth - 1, '-') << "\n";
        foreach (QVariant value, row->valueList().mid(rowIdColumns))
        {
            qOut << columns[i] + ": " + value.toString() << "\n";
            i++;
        }
        rowCnt++;
    }
    qOut.flush();
}

void CliCommandSql::executionFailed(int code, const QString& msg)
{
    UNUSED(code);
    qOut << tr("Query execution error: %1").arg(msg) << "\n\n";
    qOut.flush();
}
