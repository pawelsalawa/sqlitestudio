#include "quitconfirmdialog.h"
#include "ui_quitconfirmdialog.h"

QuitConfirmDialog::QuitConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuitConfirmDialog)
{
    init();
}

QuitConfirmDialog::~QuitConfirmDialog()
{
    delete ui;
}

void QuitConfirmDialog::addMessage(const QString& msg)
{
    ui->itemList->addItem(msg);
}

void QuitConfirmDialog::setMessages(const QStringList& messages)
{
    for (const QString& msg : messages)
        addMessage(msg);
}

int QuitConfirmDialog::getMessageCount() const
{
    return ui->itemList->count();
}

void QuitConfirmDialog::init()
{
    ui->setupUi(this);

    QStyle* style = QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize);
    QIcon icon = style->standardIcon(QStyle::SP_MessageBoxQuestion);

    if (!icon.isNull())
        ui->iconLabel->setPixmap(icon.pixmap(iconSize, iconSize));
}
