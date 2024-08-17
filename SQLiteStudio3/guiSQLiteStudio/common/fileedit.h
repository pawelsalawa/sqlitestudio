#ifndef FILEEDIT_H
#define FILEEDIT_H

#include "guiSQLiteStudio_global.h"
#include <QWidget>
#include <QVariant>

class QLineEdit;
class QToolButton;
class QComboBox;
class QAbstractItemModel;

class GUI_API_EXPORT FileEdit : public QWidget
{
        Q_OBJECT

        Q_PROPERTY(QString file READ getFile WRITE setFile NOTIFY fileChanged)
        Q_PROPERTY(bool save READ getSave WRITE setSave NOTIFY saveChanged)
        Q_PROPERTY(QString dialogTitle READ getDialogTitle WRITE setDialogTitle NOTIFY dialogTitleChanged)
        Q_PROPERTY(QString filters READ getFilters WRITE setFilters NOTIFY filtersChanged)
        Q_PROPERTY(QVariant modelName READ getChoicesModelName WRITE setChoicesModelName)

    public:
        explicit FileEdit(QWidget *parent = 0);

        QString getFile() const;
        bool getSave() const;
        QString getDialogTitle() const;
        QString getFilters() const;
        QAbstractItemModel* getChoicesModel() const;
        QVariant getChoicesModelName() const;
        void setChoicesModel(QAbstractItemModel* arg);

    private:
        QString file;
        bool save = false;
        QString dialogTitle;
        QString filters;
        QVariant choicesModelName;
        QAbstractItemModel* choicesModel = nullptr;
        QLineEdit* lineEdit = nullptr;
        QToolButton* button = nullptr;
        QComboBox* combo = nullptr;

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
        void setChoicesModelName(QVariant arg);
};

#endif // FILEEDIT_H
