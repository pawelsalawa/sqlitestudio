#ifndef CLICOMMAND_H
#define CLICOMMAND_H

#include "clicommandsyntax.h"
#include <QStringList>
#include <QTextStream>
#include <QObject>

class QFile;
class SQLiteStudio;
class DbManager;
class CLI;
class Config;

class CliCommand : public QObject
{
    Q_OBJECT

    public:
        CliCommand();
        virtual ~CliCommand();

        void setup(CLI* cli);

        /**
         * @brief execute
         * @param args Command arguments.
         */
        virtual void execute(const QStringList& args) = 0;

        /**
         * @brief Short help displayed in commands index.
         * @return Single line of command description.
         */
        virtual QString shortHelp() const = 0;

        /**
         * @brief Full help is displayed when help for specific command was requested.
         * @return Multi line, detailed description, including syntax.
         */
        virtual QString fullHelp() const = 0;

        virtual bool isAsyncExecution() const;

        virtual void defineSyntax() = 0;

        QStringList aliases() const;
        bool parseArgs(const QStringList& args);
        QString usage() const;
        QString usage(const QString& alias) const;
        QString getName() const;
        QStringList complete(const QStringList& args);

    protected:
        enum ArgIds
        {
            DB_NAME         = 1000,
            DB_NAME_OR_FILE = 1001,
            FILE_PATH       = 1002,
            PATTERN         = 1003,
            DIR_PATH        = 1004,
            CMD_NAME        = 1005,
            STRING          = 1006
        };

        static void println(const QString& str = "");
        static void printBox(const QString& str);
        static QString cmdName(const QString& cmd);

        void printUsage();
        virtual QStringList getCompletionValuesFor(int id);

        CLI* cli;
        Config* config;
        CliCommandSyntax syntax;

    signals:
        void execComplete();
};

#endif // CLICOMMAND_H
