#ifndef FILEEDIT_H
#define FILEEDIT_H

#include <QWidget>

class QLineEdit;
class QToolButton;

class FileEdit : public QWidget
{
        Q_OBJECT

        Q_PROPERTY(QString file READ getFile WRITE setFile NOTIFY fileChanged)
        Q_PROPERTY(bool save READ getSave WRITE setSave NOTIFY saveChanged)
        Q_PROPERTY(QString dialogTitle READ getDialogTitle WRITE setDialogTitle NOTIFY dialogTitleChanged)
        Q_PROPERTY(QString filters READ getFilters WRITE setFilters NOTIFY filtersChanged)

    public:
        explicit FileEdit(QWidget *parent = 0);

        QString getFile() const;
        bool getSave() const;
        QString getDialogTitle() const;
        QString getFilters() const;

    private:
        QString file;
        bool save;
        QString dialogTitle;
        QString filters;
        QLineEdit* lineEdit = nullptr;
        QToolButton* button = nullptr;

    signals:
        void fileChanged(QString arg);
        void saveChanged(bool arg);
        void dialogTitleChanged(QString arg);
        void filtersChanged(QString arg);

    private slots:
        void browse();
        void lineTextChanged();

    public slots:
        void setFile(QString arg);
        void setSave(bool arg);
        void setDialogTitle(QString arg);
        void setFilters(QString arg);
};

#endif // FILEEDIT_H
