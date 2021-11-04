#ifndef LANGUAGEDIALOG_H
#define LANGUAGEDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QMap>
#include <QDialog>

namespace Ui {
    class LanguageDialog;
}

class GUI_API_EXPORT LanguageDialog : public QDialog
{
        Q_OBJECT

    protected:
        void showEvent(QShowEvent*);

    public:
        explicit LanguageDialog(QWidget *parent = 0);
        ~LanguageDialog();

        void setLanguages(const QMap<QString, QString>& langs);
        QString getSelectedLang() const;
        void setSelectedLang(const QString& lang);

        static bool didAskForDefaultLanguage();
        static void askedForDefaultLanguage();

    private:
        Ui::LanguageDialog *ui;
};

#endif // LANGUAGEDIALOG_H
