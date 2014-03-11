#ifndef CLICOMPLETER_H
#define CLICOMPLETER_H

#include <QStringList>

class CLI;

class CliCompleter
{
    public:
        static CliCompleter* getInstance();
        static char** complete(const char* text, int start, int end);

        void init(CLI* value);

    private:
        CliCompleter();
        QStringList completeInternal(const QString& toBeReplaced, const QString& text, int curPos);
        QStringList completeCommand(const QString& str, int curPos);
        bool doKeepOriginalStr(const QString& str, int curPos);

        static char** toCharArray(const QStringList& list);

        static CliCompleter* instance;

        CLI* cli = nullptr;
};

#endif // CLICOMPLETER_H
