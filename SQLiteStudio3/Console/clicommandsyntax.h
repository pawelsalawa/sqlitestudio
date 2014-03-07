#ifndef CLICOMMANDSYNTAX_H
#define CLICOMMANDSYNTAX_H

#include <QString>
#include <QStringList>
#include <QHash>

class CliCommandSyntax
{
    public:
        CliCommandSyntax();
        ~CliCommandSyntax();

        void addArgument(int id, const QString& name, bool mandatory = true);
        void addStrictArgument(int id, const QStringList& values, bool mandatory = true);
        void addAlternatedArgument(int id, const QStringList& names, bool mandatory = true);
        void addOptionShort(int id, const QString& shortName);
        void addOptionLong(int id, const QString& longName);
        void addOption(int id, const QString& shortName, const QString& longName);
        void addOptionWithArgShort(int id, const QString& shortName, const QString& argName);
        void addOptionWithArgLong(int id, const QString& longName, const QString& argName);
        void addOptionWithArg(int id, const QString& shortName, const QString& longName, const QString& argName);

        bool parse(const QStringList& args);
        QString getErrorText() const;

        void addAlias(const QString& alias);
        QStringList getAliases() const;

        QString getSyntaxDefinition() const;
        QString getSyntaxDefinition(const QString& usedName) const;

        bool isArgumentSet(int id) const;
        QString getArgument(int id) const;

        bool isOptionSet(int id) const;
        QString getOptionValue(int id) const;

        QString getName() const;
        void setName(const QString& value);

        bool getStrictArgumentCount() const;
        void setStrictArgumentCount(bool value);

    private:
        struct Argument
        {
                enum Type
                {
                    REGULAR,
                STRICT,
                ALTERNATED
            };

            int id;
            QStringList names;
            Type type = REGULAR;
            bool mandatory = true;
            bool defined = false;
            QString value;
        };

        struct Option
        {
            int id;
            QString shortName;
            QString longName;
            QString argName;
            bool requested = false;
            QString value;
        };

        bool parseArg(const QString& arg);
        bool parseOpt(const QString& arg, const QStringList& args, int& argIdx);
        int requiredArguments() const;
        void checkNewArgument(bool mandatory);
        Argument* addArgumentInternal(int id, const QStringList& names, bool mandatory, Argument::Type type);
        Option* addOptionInternal(int id, const QString& shortName, const QString& longName, const QString& argName);

        int argPosition = 0;
        QString parsingErrorText;
        bool strictArgumentCount = true;
        QString name;
        QStringList aliases;
        QList<Argument*> arguments;
        QHash<int,Argument*> argumentMap;
        QList<Option*> options;
        QHash<int,Option*> optionMap;
        QHash<QString,Option*> optionsShortNameMap;
        QHash<QString,Option*> optionsLongNameMap;
};

#endif // CLICOMMANDSYNTAX_H
