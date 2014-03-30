#include "messagelistdialog.h"
#include "iconmanager.h"
#include "ui_messagelistdialog.h"
#include <QDebug>
#include <QLinearGradient>

MessageListDialog::MessageListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageListDialog)
{
    ui->setupUi(this);
    ui->message->setVisible(false);
}

MessageListDialog::MessageListDialog(const QString& message, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::MessageListDialog)
{
    ui->setupUi(this);
    ui->buttonBox->clear();
    ui->buttonBox->addButton(QDialogButtonBox::Yes);
    ui->buttonBox->addButton(QDialogButtonBox::No);
    ui->message->setText(message);
}

MessageListDialog::~MessageListDialog()
{
    delete ui;
}

void MessageListDialog::addMessage(const QString& text, const QBrush& background)
{
    addMessage(QIcon(), text, background);
}

void MessageListDialog::addMessage(const QIcon& icon, const QString& text, const QBrush& background)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(text);
    item->setBackground(background);
    item->setIcon(icon);
    ui->listWidget->addItem(item);
}

void MessageListDialog::addInfo(const QString& text)
{
    addMessage(ICONS.STATUS_INFO, text, getGradient(0, 0, 1, 0.2));
}

void MessageListDialog::addWarning(const QString& text)
{
    addMessage(ICONS.STATUS_WARNING, text, getGradient(0.8, 0.8, 0, 0.4));
}

void MessageListDialog::addError(const QString& text)
{
    addMessage(ICONS.STATUS_ERROR, text, getGradient(0.6, 0, 0, 0.6));
}

void MessageListDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

QBrush MessageListDialog::getGradient(qreal r, qreal g, qreal b, qreal a) const
{
    QLinearGradient gradient(0, 0, 20, 120);
    gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0));
    gradient.setColorAt(1, QColor::fromRgbF(r, g, b, a));

    return QBrush(gradient);
}

void MessageListDialog::showEvent(QShowEvent*)
{
    adjustSize();
}

void MessageListDialog::resizeEvent(QResizeEvent*)
{
    QFontMetrics metrics = ui->listWidget->fontMetrics();
    QRect rect = ui->listWidget->rect();
    int cnt = ui->listWidget->count();
    QListWidgetItem* item;
    for (int row = 0; row < cnt; row++)
    {
        item = ui->listWidget->item(row);
        item->setSizeHint(metrics.boundingRect(rect, Qt::TextWordWrap|Qt::TextLongestVariant, item->text()).size() + QSize(0, 10));
    }
}
