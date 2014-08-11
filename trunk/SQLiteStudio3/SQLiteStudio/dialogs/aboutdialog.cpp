#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common/utils.h"
#include "sqlitestudio.h"
#include <QDebug>
#include <QFile>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    init();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::init()
{
    ui->setupUi(this);

    QString distName;
    switch (getDistributionType())
    {
        case DistributionType::PORTABLE:
            distName = tr("Portable distribution.");
            break;
        case DistributionType::OS_MANAGED:
            distName = tr("Operating system managed distribution.");
            break;
    }

    QString newLabelValue = ui->aboutLabel->text().arg(SQLITESTUDIO->getVersionString(), distName);
    ui->aboutLabel->setText(newLabelValue);

    licenseContents = "";
    int row = 1;
    buildIndex();
    readMainLicense(row++);
    readIconsLicense(row++);
    licenseContents += "</ol>";

    ui->licenseEdit->setHtml(licenseContents);
}

void AboutDialog::buildIndex()
{
    licenseContents.append("<h3>Table of contents:</h3>"
                           "<ol>"
                           "<li>SQLiteStudio license (GPLv3)</li>"
                           "<li>Fugue icons</li>"
                           "</ol>");
}

void AboutDialog::readMainLicense(int row)
{
    QString rowNum = QString::number(row);
    QString contents = readFile(":/docs/license.txt");
    licenseContents += "<h3>" + rowNum + ". SQLiteStudio license (GPLv3)</h3>";
    licenseContents += "<pre>" + contents + "</pre>";
}

void AboutDialog::readIconsLicense(int row)
{
    QString rowNum = QString::number(row);
    QString contents = readFile(":/docs/licenses/fugue_icons.txt");
    licenseContents += "<h3>" + rowNum + ". Fugue icons</h3>";
    licenseContents += "<pre>" + contents + "</pre>";
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
