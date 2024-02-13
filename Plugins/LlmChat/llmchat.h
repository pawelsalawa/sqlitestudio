#ifndef LLMCHAT_H
#define LLMCHAT_H

#include <QObject>
#include <QAction>
#include "GenericPlugin.h"
#include "GeneralPurposePlugin.h"

class LlmChat : public QObject, public GenericPlugin, public GeneralPurposePlugin
{
    Q_OBJECT
    SQLITESTUDIO_PLUGIN("llmchat.json")

public:
    LlmChat();
    virtual ~LlmChat();

    bool init() override;
    void deinit() override;

private:
    QAction *llmChatAction = nullptr;
    QMainWindow *mainWindow = nullptr;

    void addToToolsMenu();
    void openLlmChatDialog();
};

#endif // LLMCHAT_H
