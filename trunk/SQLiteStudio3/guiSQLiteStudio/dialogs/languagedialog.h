#ifndef LANGUAGEDIALOG_H
#define LANGUAGEDIALOG_H

#include <QMap>
#include <QDialog>

namespace Ui {
    class LanguageDialog;
}

class LanguageDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit LanguageDialog(QWidget *parent = 0);
        ~LanguageDialog();

        void setLanguages(const QMap<QString, QString>& langs);
        QString getSelectedLang() const;
        void setSelectedLang(const QString& lang);

    private:
        Ui::LanguageDialog *ui;
};

#endif // LANGUAGEDIALOG_H
