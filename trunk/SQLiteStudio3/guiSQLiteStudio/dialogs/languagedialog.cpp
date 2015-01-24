#include "languagedialog.h"
#include "ui_languagedialog.h"

LanguageDialog::LanguageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LanguageDialog)
{
    ui->setupUi(this);
}

LanguageDialog::~LanguageDialog()
{
    delete ui;
}

void LanguageDialog::setLanguages(const QMap<QString, QString>& langs)
{
    for (const QString& langName : langs.keys())
        ui->comboBox->addItem(langName, langs[langName]);
}

QString LanguageDialog::getSelectedLang() const
{
    return ui->comboBox->currentData().toString();
}

void LanguageDialog::setSelectedLang(const QString& lang)
{
    int idx = ui->comboBox->findData(lang);
    if (idx < 0)
        idx = 0;

    ui->comboBox->setCurrentIndex(idx);
}
