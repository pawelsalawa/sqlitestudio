#include "llmchat.h"
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QDialog>
#include <QGridLayout>
#include <QNetworkRequest>

LlmChat::LlmChat(QMainWindow *parent):

    QObject(parent),
    llmChatDialog(new QDialog(parent)), // Keeps the dialog modal to the parent
    networkManager(new QNetworkAccessManager(this)){
    setupLlmChatDialog();
    connect(networkManager, &QNetworkAccessManager::finished, this, &LlmChat::handleLlmChatResponse);
}

LlmChat::~LlmChat()
{
    delete llmChatDialog;
    delete networkManager;
}
void LlmChat::setupLlmChatDialog()
{
    llmChatDialog = new QDialog(parent);
    QGridLayout* chatLayout = new QGridLayout(llmChatDialog);

    // Model selector and label
    modelSelector = new QComboBox(llmChatDialog);
    modelSelector->addItem("gpt-3.5-turbo-1106");
    modelSelector->addItem("gpt-4-0125-preview");
    chatLayout->addWidget(new QLabel(tr("Model:")), 0, 0);
    chatLayout->addWidget(modelSelector, 0, 1);

    // New Chat button setup
    newChatButton = new QPushButton(tr("New Chat"), llmChatDialog);
    newChatButton->setDefault(false);
    newChatButton->setAutoDefault(false);
    connect(newChatButton, &QPushButton::clicked, this, &LlmChat::clearChatHistory);
    chatLayout->addWidget(newChatButton, 0, 2);

    // Chat input and label, using QTextEdit for multiline input
    llmChatInput = new QTextEdit(llmChatDialog);
    llmChatInput->setFixedHeight(llmChatInput->fontMetrics().height() * 2);
    chatLayout->addWidget(new QLabel(tr("Your message:")), 2, 0);
    chatLayout->addWidget(llmChatInput, 2, 1);
    llmChatInput->installEventFilter(this);
    // Send button setup
    llmChatSendButton = new QPushButton(tr("Send"), llmChatDialog);
    llmChatSendButton->setDefault(false);
    llmChatSendButton->setAutoDefault(false);
    connect(llmChatSendButton, &QPushButton::clicked, this, &LlmChat::sendLlmChatRequest);
    chatLayout->addWidget(llmChatSendButton, 2, 2);

    // Text output for the chat
    llmChatOutput = new QTextEdit(llmChatDialog);
    llmChatOutput->setReadOnly(true);
    chatLayout->addWidget(llmChatOutput, 1, 0, 1, 3);

    // Connecting the returnPressed signal from llmChatInput to sendLlmChatRequest slot
    // This is handled through the eventFilter for QTextEdit now

    // Set layout for the dialog
    llmChatDialog->setLayout(chatLayout);
    llmChatDialog->resize(450, 490);
    
    // Connect the QDialog::rejected signal to clearChatHistory slot
    connect(llmChatDialog, &QDialog::rejected, this, &LlmChat::clearChatHistory);

    // Initialize the chat history
    chatHistory.append(QJsonObject({{"role", "system"}, {"content", "You are a helpful assistant who is an expert in sqlite and sqlitestudio. Any question related to sql you should assume relates to sqlite or sqlite studio; unless expressly stated otherwise.\n\nKeep your response concise but highly accurate. Think through the steps required to provide the request response / solution.\n\n"}}));

    llmChatDialog->show();
    llmChatInput->setFocus();
}

bool LlmChat::eventFilter(QObject *obj, QEvent *event) {
    if (obj == llmChatInput && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                QTextCursor cursor = llmChatInput->textCursor();
                cursor.insertText("\n");
                
                // Resize chat input box, maximum height for 7 lines
                int lineHeight = llmChatInput->fontMetrics().lineSpacing();
                int newHeight = qMin(lineHeight * 7, llmChatInput->height() + lineHeight);
                llmChatInput->setFixedHeight(newHeight);
                
                return true; // Event handled, do not propagate further
            } else {
                // Check if the input is not empty before sending
                if (!llmChatInput->toPlainText().trimmed().isEmpty()) {
                    sendLlmChatRequest();
                    // Reset the height of the text input box to its initial value after sending a message
                    int initialHeight = llmChatInput->fontMetrics().lineSpacing() * 2; // Assuming initial height for 2 lines
                    llmChatInput->setFixedHeight(initialHeight);
                }
                return true; // Prevent further processing
            }
        }
    }
    // Your existing file open event handling remains here unchanged
    return QObject::eventFilter(obj, event); // For events not explicitly handled above
}

void LlmChat::clearChatHistory() {
     if (!llmChatDialog) {
        setupLlmChatDialog();
    }
    llmChatDialog->show();
    llmChatInput->setFocus();
}

void LlmChat::sendLlmChatRequest() {
    QString apiKey = qgetenv("OPENAI_API_KEY");
    if (apiKey.isEmpty())
    {
        QMessageBox::warning(nullptr, tr("Error"), tr("The OpenAI API key is not set."));
        return;
    }
    // Use toPlainText() instead of text() for QTextEdit
    if (llmChatInput->toPlainText().isEmpty())
    {
        QMessageBox::information(nullptr, tr("Info"), tr("Please enter your message."));
        return;
    }

    // Similarly, use toPlainText() and then escape the HTML.
    QString userInput = llmChatInput->toPlainText().toHtmlEscaped();
    chatHistory.append(QJsonObject({{"role", "user"}, {"content", userInput}}));

    // Display user message with "You:" prefix in bold black in the UI
    llmChatOutput->append("<strong style=\"color:black;\">You:</strong> " + userInput.replace("\n", "<br>"));
    llmChatOutput->append("<br>");

    QString selectedModel = modelSelector->currentText();

    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QJsonObject json;
    json["model"] = selectedModel;
    json["messages"] = chatHistory;
    json["temperature"] = 0.5;
    json["max_tokens"] = 500;
    json["top_p"] = 1;
    json["frequency_penalty"] = 0;
    json["presence_penalty"] = 0;
    
    networkManager->post(request, QJsonDocument(json).toJson());

    llmChatInput->clear();  // Clear the input field after the request is sent
    
    // Clear the input field and set focus after sending
    llmChatInput->setFocus();
}


void LlmChat::handleLlmChatResponse(QNetworkReply* reply)
{
    if (!reply->error())
    {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QJsonObject jsonObject = jsonResponse.object();
        QJsonArray choices = jsonObject["choices"].toArray();
        if (!choices.isEmpty())
        {
            QJsonObject choice = choices.first().toObject();
            QJsonObject message = choice["message"].toObject();
            QString responseText = message["content"].toString();
            if (message["role"].toString() == "assistant")
            {
                // Append only the response text to the chat history, without the "GPT:" label
                chatHistory.append(QJsonObject({{"role", "assistant"}, {"content", responseText}}));

                // Update the UI with the response prefixed with "GPT:" in bold green
                llmChatOutput->append("<strong style=\"color:green;\">GPT:</strong> " + responseText.replace("\n", "<br>"));
                llmChatOutput->append("<br>");

            }    
        }
        else
        {
            llmChatOutput->append(tr("No response from the assistant."));
            llmChatOutput->append("<br>"); // Visual spacing
        }
    }
    else
    {
        QMessageBox::critical(nullptr, tr("Error"), reply->errorString());
    }
    reply->deleteLater();
}

