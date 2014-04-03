#include "verifiablewizardpage.h"

VerifiableWizardPage::VerifiableWizardPage(QWidget *parent) :
    QWizardPage(parent)
{
}

bool VerifiableWizardPage::isComplete() const
{
    return validator();
}

void VerifiableWizardPage::setValidator(const Validator& value)
{
    validator = value;
}

