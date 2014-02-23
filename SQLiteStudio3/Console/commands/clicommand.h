#ifndef CLICOMMAND_H
#define CLICOMMAND_H

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
        virtual void execute(QStringList args) = 0;

        /**
         * @brief validate
         * @param args Command arguments.
         * @return true if given arguments are okay for the command, or false otherwise.
         * This method should validate input arguments for the command. If they are
         * invalid it should print usage help message and return false.
         */
        virtual bool validate(QStringList args) = 0;

    protected:
        void println(const QString& str);

        DbManager* dbManager;
        CLI* cli;
        Config* config;
};

#endif // CLICOMMAND_H
