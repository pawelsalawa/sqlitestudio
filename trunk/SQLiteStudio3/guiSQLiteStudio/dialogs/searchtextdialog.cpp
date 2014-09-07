#include "searchtextdialog.h"
#include "ui_searchtextdialog.h"
#include "searchtextlocator.h"
#include "common/unused.h"

SearchTextDialog::SearchTextDialog(SearchTextLocator* textLocator, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchTextDialog), textLocator(textLocator)
{
    ui->setupUi(this);
    connect(textLocator, SIGNAL(replaceAvailable(bool)), this, SLOT(setReplaceAvailable(bool)));
}

SearchTextDialog::~SearchTextDialog()
{
    delete ui;
}

void SearchTextDialog::changeEvent(QEvent *e)
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

void SearchTextDialog::showEvent(QShowEvent* e)
{
    UNUSED(e);
    ui->findEdit->setFocus();
    ui->findEdit->selectAll();
    configModifiedState = true;
    setReplaceAvailable(false);
}

void SearchTextDialog::applyConfigToLocator()
{
    if (!configModifiedState)
        return;

    textLocator->setCaseSensitive(ui->caseSensitiveCheck->isChecked());
    textLocator->setSearchBackwards(ui->backwardsCheck->isChecked());
    textLocator->setRegularExpression(ui->regExpCheck->isChecked());
    textLocator->setLookupString(ui->findEdit->text());
    configModifiedState = false;
}

void SearchTextDialog::setReplaceAvailable(bool available)
{
    ui->replaceButton->setEnabled(available);
}

void SearchTextDialog::on_findButton_clicked()
{
    applyConfigToLocator();
    textLocator->find();
}

void SearchTextDialog::on_replaceButton_clicked()
{
    applyConfigToLocator();
    textLocator->setReplaceString(ui->replaceEdit->text());
    textLocator->replaceAndFind();
}

void SearchTextDialog::on_replaceAllButton_clicked()
{
    applyConfigToLocator();
    textLocator->setReplaceString(ui->replaceEdit->text());
    textLocator->replaceAll();
}
