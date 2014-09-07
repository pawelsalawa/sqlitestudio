#ifndef VERIFIABLEWIZARDPAGE_H
#define VERIFIABLEWIZARDPAGE_H

#include "guiSQLiteStudio_global.h"
#include <QWizardPage>

class GUI_API_EXPORT VerifiableWizardPage : public QWizardPage
{
        Q_OBJECT
    public:
        typedef std::function<bool()> Validator;

        explicit VerifiableWizardPage(QWidget *parent = 0);

        bool isComplete() const;
        void setValidator(const Validator& value);

    private:
        Validator validator;
};

#endif // VERIFIABLEWIZARDPAGE_H
