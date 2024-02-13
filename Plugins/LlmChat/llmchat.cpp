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
    qDebug() << "LlmChat::init() called.";

    mainWindow = dynamic_cast<QMainWindow *>(parent());
    if (!mainWindow)
    {
        qDebug() << "LlmChat::init() - Parent is not a QMainWindow.";
    return false;
    }

    addToToolsMenu();
    qDebug() << "LlmChat::init() completed successfully.";
    return true;
}

void LlmChat::deinit()
{
    if (mainWindow && llmChatAction)
    {
        // Iterate through menus to find the "&Tools" menu
        foreach (QAction* action, mainWindow->menuBar()->actions())
        {
            QMenu* menu = action->menu();
            if (menu && menu->title() == tr("&Tools"))
            {
                menu->removeAction(llmChatAction);
                break;
            }
        }
    }
}

void LlmChat::addToToolsMenu()
{
    llmChatAction = new QAction(QIcon(":/icons/llm_chat_icon.png"), tr("LLM &Chat"), this);
    QMenu* toolsMenu = nullptr;
    foreach (QAction* action, mainWindow->menuBar()->actions())
    {
        QMenu* menu = action->menu();
        if (menu && menu->title() == tr("&Tools"))
        {
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