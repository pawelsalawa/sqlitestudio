#include "dbconverterdialog.h"
#include "ui_dbconverterdialog.h"
#include "common/global.h"
#include "dblistmodel.h"
#include "db/db.h"
#include "dbversionconverter.h"
#include "services/dbmanager.h"
#include "iconmanager.h"
#include <QFileInfo>
#include <QDir>

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
    setWindowTitle(tr("Convert database"));

    ui->trgFileButton->setIcon(ICONS.OPEN_FILE);

    converter = new DbVersionConverter();

    dbListModel = new DbListModel(this);
    ui->srcDbCombo->setModel(dbListModel);

    connect(ui->srcDbCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(srcDbChanged(int)));
}

void DbConverterDialog::srcDbChanged()
{
    ui->srcDbVersionCombo->clear();
    ui->trgVersionCombo->clear();
    if (srcDb)
    {
        // Source version
        Dialect dialect = srcDb->getDialect();
        QList<Dialect> dialects = converter->getSupportedVersions();
        int idx = dialects.indexOf(dialect);
        QStringList versionNames = converter->getSupportedVersionNames();
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
}

void DbConverterDialog::srcDbChanged(int index)
{
    srcDb = dbListModel->getDb(index);
    srcDbChanged();
}
