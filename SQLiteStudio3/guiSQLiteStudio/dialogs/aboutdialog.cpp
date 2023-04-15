#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common/utils.h"
#include "iconmanager.h"
#include "services/extralicensemanager.h"
#include "services/pluginmanager.h"
#include "services/sqliteextensionmanager.h"
#include "formmanager.h"
#include "iconmanager.h"
#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QApplication>
#include <QClipboard>
#include <QAction>

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

    // About
    QString distName;
    switch (getDistributionType())
    {
        case DistributionType::PORTABLE:
            distName = tr("Portable distribution.");
            break;
        case DistributionType::OSX_BUNDLE:
            distName = tr("MacOS X application bundle distribution.");
            break;
        case DistributionType::OS_MANAGED:
            distName = tr("Operating system managed distribution.");
            break;
    }

    QString newLabelValue = ui->aboutLabel->text().arg(SQLITESTUDIO->getVersionString(), distName);
    ui->aboutLabel->setText(newLabelValue);

    // Licenses
    licenseContents = "";
    int row = 1;

    QHash<QString,QString> licenses = SQLITESTUDIO->getExtraLicenseManager()->getLicensesContents();
    QString violation;
    QString title;
    QHashIterator<QString,QString> it(licenses);
    while (it.hasNext())
    {
        it.next();
        violation = QString();
        title = it.key();
        if (SQLITESTUDIO->getExtraLicenseManager()->isViolatedLicense(title))
            violation = SQLITESTUDIO->getExtraLicenseManager()->getViolationMessage(title);

        addLicense(row++, title, it.value(), violation);
    }

    buildIndex();

    ui->licenseEdit->setHtml(licenseContents);
    indexContents.clear();
    licenseContents.clear();

    // Environment
    ui->appDirEdit->setText(toNativePath(qApp->applicationDirPath()));
    ui->cfgDirEdit->setText(toNativePath(CFG->getConfigDir()));
    ui->pluginDirList->setPlainText(filterResourcePaths(PLUGINS->getPluginDirs()).join("\n"));
    ui->iconDirList->setPlainText(filterResourcePaths(ICONMANAGER->getIconDirs()).join("\n"));
    ui->formDirList->setPlainText(filterResourcePaths(FORMS->getFormDirs()).join("\n"));
    ui->extensionDirList->setPlainText(filterResourcePaths(SQLITE_EXTENSIONS->getExtensionDirs()).join("\n"));
    ui->qtVerEdit->setText(QT_VERSION_STR);
    ui->sqlite3Edit->setText(CFG->getSqlite3Version());
}

void AboutDialog::buildIndex()
{
    static const QString entryTpl = QStringLiteral("<li>%1</li>");
    QStringList entries;
    for (QString& idx : indexContents)
        entries += entryTpl.arg(idx);

    licenseContents.prepend(tr("<h3>Table of contents:</h3><ol>%2</ol>").arg(entries.join("")));
}

void AboutDialog::addLicense(int row, const QString& title, const QString& contents, const QString& violation)
{
    static_qstring(violatedTpl, "<span style=\"color: #FF0000;\">%1 (%2)</span>");

    QString escapedTitle = title.toHtmlEscaped();
    QString finalTitle = violation.isNull() ? escapedTitle : violatedTpl.arg(escapedTitle, violation);
    QString rowNum = QString::number(row);
    licenseContents += "<h3>" + rowNum + ". " + finalTitle + "</h3>";
    licenseContents += "<pre>" + contents.toHtmlEscaped() + "</pre>";
    indexContents += finalTitle;
}

QString AboutDialog::readFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << "Error opening" << file.fileName();
        return QString();
    }
    QString contents = QString::fromLatin1(file.readAll()).toHtmlEscaped();
    file.close();
    return contents;
}

QStringList AboutDialog::filterResourcePaths(const QStringList& paths)
{
    QStringList output;
    for (const QString& path : paths)
    {
        if (path.startsWith(":"))
            continue;

        QString newPath = toNativePath(path);
        output << newPath;
    }
    return output;
}
