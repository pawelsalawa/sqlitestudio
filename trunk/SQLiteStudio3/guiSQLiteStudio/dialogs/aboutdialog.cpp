#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common/utils.h"
#include "sqlitestudio.h"
#include "iconmanager.h"
#include "services/extralicensemanager.h"
#include <QDebug>
#include <QFile>

AboutDialog::AboutDialog(InitialMode initialMode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    init(initialMode);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::init(InitialMode initialMode)
{
    ui->setupUi(this);
    ui->leftIcon->setPixmap(ICONS.SQLITESTUDIO_APP.toQIcon().pixmap(200, 200));

    ui->tabWidget->setCurrentWidget(initialMode == ABOUT ? ui->about : ui->license);

    QString distName;
    switch (getDistributionType())
    {
        case DistributionType::PORTABLE:
            distName = tr("Portable distribution.");
            break;
        case DistributionType::OSX_BOUNDLE:
            distName = tr("MacOS X application boundle distribution.");
            break;
        case DistributionType::OS_MANAGED:
            distName = tr("Operating system managed distribution.");
            break;
    }

    QString newLabelValue = ui->aboutLabel->text().arg(SQLITESTUDIO->getVersionString(), distName);
    ui->aboutLabel->setText(newLabelValue);

    licenseContents = "";
    int row = 1;

    QHash<QString,QString> licenses = SQLITESTUDIO->getExtraLicenseManager()->getLicenses();
    QHashIterator<QString,QString> it(licenses);
    while (it.hasNext())
    {
        it.next();
        readLicense(row++, it.key(), it.value());
    }

    buildIndex();

    ui->licenseEdit->setHtml(licenseContents);
    indexContents.clear();
    licenseContents.clear();
}

void AboutDialog::buildIndex()
{
    static const QString entryTpl = QStringLiteral("<li>%1</li>");
    QStringList entries;
    for (const QString& idx : indexContents)
        entries += entryTpl.arg(idx);

    licenseContents.prepend("<h3>Table of contents:</h3><ol>" + entries.join("") + "</ol>");
}

void AboutDialog::readLicense(int row, const QString& title, const QString& path)
{
    QString rowNum = QString::number(row);
    QString contents = readFile(path);
    licenseContents += "<h3>" + rowNum + ". " + title + "</h3>";
    licenseContents += "<pre>" + contents + "</pre>";
    indexContents += title;
}

QString AboutDialog::readFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Error opening" << file.fileName();
        return QString::null;
    }
    QString contents = QString::fromLatin1(file.readAll()).toHtmlEscaped();
    file.close();
    return contents;
}
