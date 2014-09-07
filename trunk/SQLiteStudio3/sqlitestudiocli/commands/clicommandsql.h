#ifndef CLICOMMANDSQL_H
#define CLICOMMANDSQL_H

#include "clicommand.h"
#include "db/sqlquery.h"

class QueryExecutor;

class CliCommandSql : public CliCommand
{
        Q_OBJECT

    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        bool isAsyncExecution() const;
        void defineSyntax();

    private:
        class SortedColumnWidth
        {
            public:
                SortedColumnWidth();

                bool operator<(const SortedColumnWidth& other);

                int getHeaderWidth() const;
                void setHeaderWidth(int value);
                void setMaxHeaderWidth(int value);
                void incrHeaderWidth(int value = 1);
                void decrHeaderWidth(int value = 1);

                int getDataWidth() const;
                void setDataWidth(int value);
                void setMinDataWidth(int value);
                void incrDataWidth(int value = 1);
                void decrDataWidth(int value = 1);

                void incrWidth(int value = 0);
                int getWidth() const;
                bool isHeaderLonger() const;

            private:
                void updateWidth();

                int width;
                int headerWidth;
                int dataWidth;
        };

        void printResultsClassic(QueryExecutor *executor, SqlQueryPtr results);
        void printResultsFixed(QueryExecutor *executor, SqlQueryPtr results);
        void printResultsColumns(QueryExecutor *executor, SqlQueryPtr results);
        void printResultsRowByRow(QueryExecutor *executor, SqlQueryPtr results);
        void shrinkColumns(QList<SortedColumnWidth*>& columnWidths, int termCols, int resultColumnsCount, int totalWidth);
        void printColumnHeader(const QList<int>& widths, const QStringList& columns);
        void printColumnDataRow(const QList<int>& widths, const SqlResultsRowPtr& row, int rowIdOffset);

        QString getValueString(const QVariant& value);

    private slots:
        void executionFailed(int code, const QString& msg);
};

#endif // CLICOMMANDSQL_H
