#ifndef CLICOMMANDEXECUTOR_H
#define CLICOMMANDEXECUTOR_H

#include <QObject>
#include <QStringList>

class CliCommand;

class CliCommandExecutor : public QObject
{
    Q_OBJECT

    public:
        explicit CliCommandExecutor(QObject *parent = 0);

    signals:
        void executionComplete();

    public slots:
        void execCommand(CliCommand* cmd, QStringList args);

    private slots:
        void asyncExecutionComplete();
};

#endif // CLICOMMANDEXECUTOR_H
