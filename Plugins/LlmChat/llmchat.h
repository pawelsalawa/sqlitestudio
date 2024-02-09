#ifndef LLMCHAT_H
#define LLMCHAT_H

#include <QObject>
#include <QJsonArray>
// Forward declarations for Qt classes used
class QNetworkRequest;
class QMessageBox;
class QJsonDocument;
class QJsonObject;
class QJsonArray;
class QLabel;
class QGridLayout;
class QDialog;
class QTextEdit;
class QPushButton;
class QComboBox;
class QNetworkReply;
class QNetworkAccessManager;
class QNetworkRequest;
class LlmChat: public QObject 
{
    Q_OBJECT

public:
    explicit LlmChat(QMainWindow *parent = nullptr);
    virtual ~LlmChat();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QDialog* llmChatDialog;
    QTextEdit* llmChatInput, *llmChatOutput;
    QPushButton* llmChatSendButton, *newChatButton;
    QComboBox* modelSelector;
    QJsonArray* chatHistory;
    QNetworkAccessManager* networkManager;

    void setupLlmChatDialog();
    void sendLlmChatRequest();
    void clearChatHistory();

public slots:
    void setupLlmChatDialog();
    void handleLlmChatResponse(QNetworkReply* reply);
};

#endif // LLMCHAT_H