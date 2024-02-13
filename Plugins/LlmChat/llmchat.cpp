#include "LlmChat.h"
#include <QMenuBar>
#include <QMainWindow>
#include <QDebug>

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
    QMenu* toolsMenu = nullptr;
    foreach (QAction* action, mainWindow->menuBar()->actions()) {
        QMenu* menu = action->menu();
        if (menu && menu->title() == tr("&Tools")) {
            toolsMenu = menu;
            break;
        }
    }

    if (toolsMenu) {
        toolsMenu->addAction(llmChatAction);
    } else {
        // Log an error if the "&Tools" menu does not exist
        qWarning() << "LLM Chat plugin error: '&Tools' menu not found in the main window.";
    }
}

