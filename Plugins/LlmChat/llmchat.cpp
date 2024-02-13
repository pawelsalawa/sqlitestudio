#include "LlmChat.h"
#include <QMenuBar>
#include <QMainWindow>

LlmChat::LlmChat() : llmChatAction(nullptr), mainWindow(nullptr)
{
}

LlmChat::~LlmChat()
{
    delete llmChatAction;
}

bool LlmChat::init()
{
    mainWindow = dynamic_cast<QMainWindow *>(parent());
    if (!mainWindow) return false;

    addToToolsMenu();

    return true;
}

void LlmChat::deinit()
{
    // This would remove the action from the menu and clean up
    if (mainWindow && llmChatAction) {
        auto menu = mainWindow->menuBar()->findMenu(tr("&Tools"));
        if (menu) {
            menu->removeAction(llmChatAction);
        }
    }
}

void LlmChat::addToToolsMenu()
{
    llmChatAction = new QAction(QIcon(":/llm_chat_icon.png"), tr("LLM &Chat"), this);
    //connect(llmChatAction, &QAction::triggered, this, &LlmChat::openLlmChatDialog);

    auto toolsMenu = mainWindow->menuBar()->findMenu(tr("&Tools"));
    if (toolsMenu) {
        toolsMenu->addAction(llmChatAction);
    }
}

void LlmChat::openLlmChatDialog()
{
    // Here you should implement the logic to open the LLM Chat dialog
}
