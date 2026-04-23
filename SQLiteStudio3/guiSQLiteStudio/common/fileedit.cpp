#include "fileedit.h"
#include "iconmanager.h"
#include "uiconfig.h"
#include "common/collections.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QFileDialog>

FileEdit::FileEdit(QWidget *parent) :
    QWidget(parent)
{
    setLayout(new QHBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);

    lineEdit = new QLineEdit();
    button = new QToolButton();
    button->setIcon(ICONS.OPEN_FILE);
    layout()->addWidget(lineEdit);
    layout()->addWidget(button);

    connect(button, SIGNAL(clicked()), this, SLOT(browse()));
    connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(lineTextChanged()));
}

QString FileEdit::getFile() const
{
    return file;
}

bool FileEdit::getSave() const
{
    return save;
}

QString FileEdit::getDialogTitle() const
{
    return dialogTitle;
}

QString FileEdit::getFilters() const
{
    return filters;
}

QAbstractItemModel* FileEdit::getChoicesModel() const
{
    return choicesModel;
}

QVariant FileEdit::getChoicesModelName() const
{
    return choicesModelName;
}

void FileEdit::browse()
{
    selectedFilter = nullptr;
    if (filters.contains(";;"))
    {
        QStringList filterList = filters.split(";;");
        selectedFilter = &filterList[0];
    }

    QString path;
    QString dir = getFileDialogInitPath();
    if (save)
        path = QFileDialog::getSaveFileName(this, dialogTitle, dir, filters, selectedFilter);
    else
        path = QFileDialog::getOpenFileName(this, dialogTitle, dir, filters, selectedFilter);

    if (path.isNull())
        return;

    QString ext = getSelectedFilterExtensionIfMissing(path);
    if (!ext.isEmpty())
        path += "." + ext;

    setFile(path);
    setFileDialogInitPathByFile(path);
}

void FileEdit::lineTextChanged()
{
    file = lineEdit->text();
    emit fileChanged(file);
}

void FileEdit::setFile(QString arg)
{
    if (file != arg) {
        file = arg;
        lineEdit->setText(file);
        emit fileChanged(arg);
    }
}

void FileEdit::setSave(bool arg)
{
    if (save != arg) {
        save = arg;
        emit saveChanged(arg);
    }
}

void FileEdit::setDialogTitle(QString arg)
{
    if (dialogTitle != arg) {
        dialogTitle = arg;
        emit dialogTitleChanged(arg);
    }
}

void FileEdit::setFilters(QString arg)
{
    if (filters != arg) {
        filters = arg;
        emit filtersChanged(arg);
    }
}

void FileEdit::setChoicesModel(QAbstractItemModel* arg)
{
    if (choicesModel != arg)
    {
        disconnect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(lineTextChanged()));
        choicesModel = arg;
        if (choicesModel != nullptr && combo == nullptr)
        {
            combo = new QComboBox();
            combo->setEditable(true);
            layout()->replaceWidget(lineEdit, combo);
            delete lineEdit;
            lineEdit = combo->lineEdit();
        }
        else if (choicesModel == nullptr && combo != nullptr)
        {
            lineEdit = new QLineEdit();
            lineEdit->setText(file);
            layout()->replaceWidget(combo, lineEdit);
            delete combo;
            combo = nullptr;
        }
        if (combo != nullptr)
        {
            combo->setModel(choicesModel);
            lineEdit->setText(file);
        }
        connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(lineTextChanged()));
    }
}

QString FileEdit::getSelectedFilterExtensionIfMissing(const QString& path) const
{
    if (!selectedFilter)
        return QString();

    int idx = selectedFilter->lastIndexOf("(");
    if (idx < 0)
        return QString();

    QStringList extensions = selectedFilter->mid(idx + 1).chopped(1).split(",")
                            | MAP(e, {return e.mid(2);});
    if (extensions.isEmpty())
        return QString();

    QString lowerPath = path.toLower();
    for (QString& ext : extensions)
    {
        if (lowerPath.endsWith(ext.toLower()))
            return QString();
    }

    return extensions.first();
}

void FileEdit::setChoicesModelName(QVariant arg)
{
    choicesModelName = arg;
}
