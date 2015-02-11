#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common/utils.h"
#include "sqlitestudio.h"
#include "iconmanager.h"
#include "services/extralicensemanager.h"
#include "services/pluginmanager.h"
#include "formmanager.h"
#include "iconmanager.h"
#include <QDebug>
#include <QFile>
#include <QApplication>
#include <QClipboard>

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
        case DistributionType::OSX_BOUNDLE:
            distName = tr("MacOS X application boundle distribution.");
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
    ui->appDirEdit->setText(qApp->applicationDirPath());
    ui->cfgDirEdit->setText(CFG->getConfigDir());
    ui->pluginDirList->addItems(filterResourcePaths(PLUGINS->getPluginDirs()));
    ui->iconDirList->addItems(filterResourcePaths(ICONMANAGER->getIconDirs()));
    ui->formDirList->addItems(filterResourcePaths(FORMS->getFormDirs()));
    ui->qtVerEdit->setText(QT_VERSION_STR);
    ui->sqlite3Edit->setText(CFG->getSqlite3Version());

    QAction* copyAct;
    for (QListWidget* w : {ui->pluginDirList, ui->iconDirList, ui->formDirList})
    {
        copyAct = new QAction(tr("Copy"), w);
        w->addAction(copyAct);
        connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));
    }
}

void AboutDialog::buildIndex()
{
    static const QString entryTpl = QStringLiteral("<li>%1</li>");
    QStringList entries;
    for (const QString& idx : indexContents)
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
        return QString::null;
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

        output << path;
    }
    return output;
}

void AboutDialog::copy()
{
    QListWidget* list = dynamic_cast<QListWidget*>(sender()->parent());
    if (!list)
        return;

    QList<QListWidgetItem*> items = list->selectedItems();
    if (items.size() == 0)
        return;

    QStringList lines;
    for (QListWidgetItem* item : items)
        lines << item->text();

    QApplication::clipboard()->setText(lines.join("\n"));
}
