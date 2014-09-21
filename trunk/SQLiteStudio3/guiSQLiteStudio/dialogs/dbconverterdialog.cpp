#include "dbconverterdialog.h"
#include "ui_dbconverterdialog.h"
#include "common/global.h"
#include "dblistmodel.h"
#include "db/db.h"
#include "common/utils_sql.h"
#include "dbversionconverter.h"
#include "services/dbmanager.h"
#include "iconmanager.h"
#include "uiutils.h"
#include "versionconvertsummarydialog.h"
#include "mainwindow.h"
#include "errorsconfirmdialog.h"
#include "parser/ast/sqlitecreatetable.h"
#include "services/pluginmanager.h"
#include "plugins/dbplugin.h"
#include "db/sqlquery.h"
#include "services/notifymanager.h"
#include "common/widgetcover.h"
#include <QDebug>
#include <QFileInfo>
#include <QPushButton>

DbConverterDialog::DbConverterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DbConverterDialog)
{
    init();
}

DbConverterDialog::~DbConverterDialog()
{
    delete ui;
    safe_delete(converter);
}

void DbConverterDialog::setDb(Db* db)
{
    ui->srcDbCombo->setCurrentText(db->getName());
    srcDb = db;
    srcDbChanged();
}

void DbConverterDialog::init()
{
    ui->setupUi(this);
    limitDialogWidth(this);
    setWindowTitle(tr("Convert database"));

    widgetCover = new WidgetCover(this);
    widgetCover->setVisible(false);
    widgetCover->initWithInterruptContainer();

    ui->trgFileButton->setIcon(ICONS.OPEN_FILE);

    converter = new DbVersionConverter();

    dbListModel = new DbListModel(this);
    ui->srcDbCombo->setModel(dbListModel);

    connect(ui->srcDbCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(srcDbChanged(int)));
    connect(ui->trgVersionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateState()));
    connect(ui->trgFileEdit, SIGNAL(textChanged(QString)), this, SLOT(updateState()));
    connect(ui->trgNameEdit, SIGNAL(textChanged(QString)), this, SLOT(updateState()));
    connect(converter, SIGNAL(conversionFailed(QString)), this, SLOT(processingFailed(QString)));
    connect(converter, SIGNAL(conversionSuccessful()), this, SLOT(processingSuccessful()));
    connect(converter, SIGNAL(conversionAborted()), this, SLOT(processingAborted()));
    connect(widgetCover, SIGNAL(cancelClicked()), converter, SLOT(interrupt()));
}

void DbConverterDialog::srcDbChanged()
{
    dontUpdateState = true;
    ui->srcDbVersionCombo->clear();
    ui->trgVersionCombo->clear();
    if (srcDb)
    {
        // Source version
        QList<Dialect> dialects = converter->getSupportedVersions();
        QStringList versionNames = converter->getSupportedVersionNames();
        Dialect dialect = srcDb->getDialect();
        int idx = dialects.indexOf(dialect);
        QString type = versionNames[idx];
        ui->srcDbVersionCombo->addItem(type);
        ui->srcDbVersionCombo->setCurrentText(type);

        // Target version
        QString oldTrgVersion = ui->trgVersionCombo->currentText();
        versionNames.removeAt(idx);
        ui->trgVersionCombo->addItems(versionNames);
        if (versionNames.contains(oldTrgVersion))
            ui->trgVersionCombo->setCurrentText(oldTrgVersion);
        else if (versionNames.size() > 0)
            ui->trgVersionCombo->setCurrentIndex(0);

        // File
        QString trgFile = srcDb->getPath() + "_new";
        int i = 0;
        while (QFileInfo(trgFile).exists())
        {
            trgFile = srcDb->getPath() + "_new" + QString::number(i++);
        }

        ui->trgFileEdit->setText(trgFile);

        // Name
        QString generatedName = generateUniqueName(srcDb->getName() + "_new", DBLIST->getDbNames());
        ui->trgNameEdit->setText(generatedName);
    }
    else
    {
        ui->srcDbVersionCombo->setCurrentText("");
        ui->trgFileEdit->setText("");
        ui->trgVersionCombo->setCurrentText("");
        ui->trgNameEdit->setText("");
    }
    dontUpdateState = false;
    updateState();
}

bool DbConverterDialog::validate()
{
    bool srcDbOk = (srcDb != nullptr);
    setValidState(ui->srcDbCombo, srcDbOk, tr("Select source database"));

    QString dstDbPath = ui->trgFileEdit->text();
    QFileInfo dstDbFi(dstDbPath);
    bool dstDbOk = (!dstDbFi.exists() || dstDbFi.isWritable()) && dstDbFi != QFileInfo(srcDb->getPath());
    bool dstExists = dstDbFi.exists();
    setValidState(ui->trgFileEdit, dstDbOk, tr("Enter valid and writable file path."));
    if (dstExists && dstDbOk)
        setValidStateInfo(ui->trgFileEdit, tr("Entered file exists and will be overwritten."));

    QString name = ui->trgNameEdit->text();
    bool nameOk = !name.isEmpty() && !DBLIST->getDbNames().contains(name);
    setValidState(ui->trgNameEdit, nameOk, tr("Enter a not empty, unique name (as in the list of databases on the left)."));

    bool dstDialectOk = ui->trgVersionCombo->currentIndex() > -1;
    QString msg;
    if (!dstDialectOk && ui->trgVersionCombo->count() == 0)
        msg = tr("No valid target dialect available. Conversion not possible.");
    else
        msg = tr("Select valid target dialect.");

    setValidState(ui->trgVersionCombo, dstDialectOk, msg);

    return (srcDbOk && nameOk && dstDbOk && dstDialectOk);
}

void DbConverterDialog::accept()
{
    if (!validate())
        return;

    QStringList versionNames = converter->getSupportedVersionNames();
    QList<Dialect> dialects = converter->getSupportedVersions();
    QString trgDialectName = ui->trgVersionCombo->currentText();
    int idx = versionNames.indexOf(trgDialectName);
    if (idx == -1)
    {
        qCritical() << "Could not find target dialect on list of supported dialects in DbConverterDialog::accept()";
        return;
    }

    Dialect srcDialect = srcDb->getDialect();
    Dialect trgDialect = dialects[idx];
    QString trgFile = ui->trgFileEdit->text();
    QString trgName = ui->trgNameEdit->text();
    widgetCover->show();
    converter->convert(srcDialect, trgDialect, srcDb, trgFile, trgName, &DbConverterDialog::confirmConversion, &DbConverterDialog::confirmConversionErrors);
}

void DbConverterDialog::srcDbChanged(int index)
{
    srcDb = dbListModel->getDb(index);
    srcDbChanged();
}

void DbConverterDialog::updateState()
{
    if (dontUpdateState)
        return;

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validate());
}

void DbConverterDialog::processingFailed(const QString& errorMessage)
{
    widgetCover->hide();
    notifyError(errorMessage);
}

void DbConverterDialog::processingSuccessful()
{
    notifyInfo(tr("Database %1 has been successfully converted and now is available under new name: %2").arg(srcDb->getName(), ui->trgNameEdit->text()));
    QDialog::accept();
}

void DbConverterDialog::processingAborted()
{
    widgetCover->hide();
}

bool DbConverterDialog::confirmConversion(const QList<QPair<QString, QString> >& diffs)
{
    VersionConvertSummaryDialog dialog(MAINWINDOW);
    dialog.setWindowTitle(tr("SQL statements conversion"));
    dialog.setSides(diffs);
    return dialog.exec() == QDialog::Accepted;
}

bool DbConverterDialog::confirmConversionErrors(const QSet<QString>& errors)
{
    ErrorsConfirmDialog dialog(MAINWINDOW);
    dialog.setTopLabel(tr("Following error occurred while converting SQL statements to the target SQLite version:"));
    dialog.setBottomLabel(tr("Would you like to ignore those errors and proceed?"));
    dialog.setErrors(errors);
    return dialog.exec() == QDialog::Accepted;
}
