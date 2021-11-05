#include "iconmanager.h"
#include "languagedialog.h"
#include "ui_languagedialog.h"
#include "uiconfig.h"

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

bool LanguageDialog::didAskForDefaultLanguage()
{
    return CFG_UI.General.LanguageAsked.get();
}

void LanguageDialog::askedForDefaultLanguage()
{
    CFG_UI.General.LanguageAsked.set(true);
}

void LanguageDialog::showEvent(QShowEvent*)
{
    setWindowIcon(ICONS.SQLITESTUDIO_APP);
}
