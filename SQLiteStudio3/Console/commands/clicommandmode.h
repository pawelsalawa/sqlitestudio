#ifndef CLICOMMANDMODE_H
#define CLICOMMANDMODE_H

#include "clicommand.h"

class CliCommandMode : public CliCommand
{
    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    protected:
        QStringList getCompletionValuesFor(int id, const QString& partialValue);

    private:
        enum ArgIgs
        {
            MODE
        };
};

#endif // CLICOMMANDMODE_H
