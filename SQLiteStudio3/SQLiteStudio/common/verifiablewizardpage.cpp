#include "verifiablewizardpage.h"

VerifiableWizardPage::VerifiableWizardPage(QWidget *parent) :
    QWizardPage(parent)
{
}

bool VerifiableWizardPage::isComplete() const
{
    if (!validator)
        return false;

    return validator();
}

void VerifiableWizardPage::setValidator(const Validator& value)
{
    validator = value;
}

