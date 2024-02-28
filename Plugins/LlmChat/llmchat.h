#ifndef LLMCHAT_H
#define LLMCHAT_H

#include <QAction>
#include "plugins/genericplugin.h"

class QMainWindow;

class LlmChat : public GenericPlugin
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