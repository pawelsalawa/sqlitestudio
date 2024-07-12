#include "importdialog.h"
#include "dblistmodel.h"
#include "dbobjlistmodel.h"
#include "uiutils.h"
#include "common/widgetcover.h"
#include "services/dbmanager.h"
#include "plugins/importplugin.h"
#include "ui_importdialog.h"
#include "configmapper.h"
#include "formmanager.h"
#include "common/utils.h"
#include "uiconfig.h"
#include "themetuner.h"
#include "iconmanager.h"
#include "mainwindow.h"
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>

static const QString IMPORT_DIALOG_CFG_GROUP = "ImportDialog";
static const QString IMPORT_DIALOG_CFG_CODEC = "codec";
static const QString IMPORT_DIALOG_CFG_FILE = "inputFileName";
static const QString IMPORT_DIALOG_CFG_IGNORE_ERR = "ignoreErrors";
static const QString IMPORT_DIALOG_CFG_FORMAT = "format";

ImportDialog::ImportDialog(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::ImportDialog)
{
    init();
}

ImportDialog::~ImportDialog()
{
    IMPORT_MANAGER->interrupt();
    safe_delete(configMapper);
    delete ui;
}

void ImportDialog::setDbAndTable(Db* db, const QString& table)
{
    if (!db)
        return;

    ui->dbNameCombo->setCurrentText(db->getName());
    ui->tableNameCombo->setCurrentText(table);
}

void ImportDialog::setDb(Db* db)
{
    if (!db)
        return;

    ui->dbNameCombo->setCurrentText(db->getName());
}

bool ImportDialog::isPluginConfigValid() const
{
    return pluginConfigOk.size() == 0;
}

void ImportDialog::storeStdConfig(ImportManager::StandardImportConfig &stdConfig)
{
    CFG->begin();
    CFG->set(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_CODEC, stdConfig.codec);
    CFG->set(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_FILE, stdConfig.inputFileName);
    CFG->set(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_IGNORE_ERR, stdConfig.ignoreErrors);
    CFG->set(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_FORMAT, currentPlugin->getDataSourceTypeName());
    CFG->commit();
}

void ImportDialog::readStdConfig()
{
    QString format = CFG->get(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_FORMAT).toString();
    int idx = ui->dsTypeCombo->findText(format);
    if (idx > -1)
        ui->dsTypeCombo->setCurrentIndex(idx);

    ui->inputFileEdit->setText(CFG->get(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_FILE, QString()).toString());
    ui->ignoreErrorsCheck->setChecked(CFG->get(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_IGNORE_ERR, false).toBool());

    // Encoding
    QString codec = CFG->get(IMPORT_DIALOG_CFG_GROUP, IMPORT_DIALOG_CFG_CODEC).toString();
    QString defaultCodec = defaultCodecName();
    if (codec.isNull())
        codec = defaultCodec;

    int codecIdx = ui->codecCombo->findText(codec);
    if (codecIdx == -1 && codec != defaultCodec)
    {
        codec = defaultCodec;
        codecIdx = ui->codecCombo->findText(codec);
    }

    if (codecIdx > -1)
        ui->codecCombo->setCurrentIndex(codecIdx);
}

void ImportDialog::init()
{
    ui->setupUi(this);
    THEME_TUNER->darkThemeFix(this);
    limitDialogWidth(this);

#ifdef Q_OS_MACX
    resize(width() + 150, height());
    setPixmap(QWizard::BackgroundPixmap, addOpacity(ICONS.DATABASE_IMPORT_WIZARD.toQIcon().pixmap(800, 800), 0.3));
#endif

    initTablePage();
    initDataSourcePage();

    widgetCover = new WidgetCover(this);
    widgetCover->initWithInterruptContainer(tr("Cancel"));
    connect(widgetCover, SIGNAL(cancelClicked()), IMPORT_MANAGER, SLOT(interrupt()));
    widgetCover->setVisible(false);

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged()));
    connect(IMPORT_MANAGER, SIGNAL(validationResultFromPlugin(bool,CfgEntry*,QString)), this, SLOT(handleValidationResultFromPlugin(bool,CfgEntry*,QString)));
    connect(IMPORT_MANAGER, SIGNAL(stateUpdateRequestFromPlugin(CfgEntry*,bool,bool)), this, SLOT(stateUpdateRequestFromPlugin(CfgEntry*,bool,bool)));
    connect(IMPORT_MANAGER, SIGNAL(importSuccessful()), this, SLOT(success()));
    connect(IMPORT_MANAGER, SIGNAL(importFinished()), this, SLOT(hideCoverWidget()));
}

void ImportDialog::initTablePage()
{
    dbListModel = new DbListModel(this);
    dbListModel->setCombo(ui->dbNameCombo);
    dbListModel->setSortMode(DbListModel::SortMode::AlphabeticalCaseInsensitive);
    ui->dbNameCombo->setModel(dbListModel);

    tablesModel = new DbObjListModel(this);
    tablesModel->setIncludeSystemObjects(false);
    tablesModel->setType(DbObjListModel::ObjectType::TABLE);
    tablesModel->setSortMode(DbObjListModel::SortMode::AlphabeticalCaseInsensitive);
    ui->tableNameCombo->setModel(tablesModel);
    refreshTables();

    connect(ui->dbNameCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(refreshTables()));
    connect(ui->tableNameCombo, SIGNAL(currentTextChanged(QString)), ui->tablePage, SIGNAL(completeChanged()));

    ui->tablePage->setValidator([=]() -> bool
    {
        bool valid = !ui->tableNameCombo->currentText().isEmpty();
        setValidStateWihtTooltip(ui->tableNameCombo, tr("If you type table name that doesn't exist, it will be created."), valid, tr("Enter the table name"));
        return valid;
    });
}

void ImportDialog::initDataSourcePage()
{
    ui->inputFileButton->setIcon(ICONS.OPEN_FILE);
    connect(ui->inputFileButton, SIGNAL(clicked()), this, SLOT(browseForInputFile()));

    ui->codecCombo->addItems(textCodecNames());
    ui->codecCombo->setCurrentText(defaultCodecName());

    ui->dsPage->setValidator([=]() -> bool
    {
        setValidState(ui->dsTypeCombo, true);
        if (!currentPlugin)
        {
            setValidState(ui->dsTypeCombo, false, tr("Select import plugin."));
            return false;
        }

        if (currentPlugin->standardOptionsToEnable().testFlag(ImportManager::FILE_NAME))
        {
            QString path = ui->inputFileEdit->text();
            if (path.trimmed().isEmpty())
            {
                setValidState(ui->inputFileEdit, false, tr("You must provide a file to import from."));
                return false;
            }

            QFileInfo file(path);
            if (!file.exists())
            {
                setValidState(ui->inputFileEdit, false, tr("The file '%1' does not exist.").arg(path));
                return false;
            }

            if (file.exists() && file.isDir())
            {
                setValidState(ui->inputFileEdit, false, tr("Path you provided is a directory. A regular file is required."));
                return false;
            }
            setValidState(ui->inputFileEdit, true);
        }
        return ui->dsTypeCombo->currentIndex() > -1 && ui->codecCombo->currentIndex() > -1 && isPluginConfigValid();
    });

    connect(this, SIGNAL(dsPageCompleteChanged()), ui->dsPage, SIGNAL(completeChanged()));
    connect(ui->dsTypeCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(pluginSelected()));
    connect(ui->dsTypeCombo, SIGNAL(currentTextChanged(QString)), ui->dsPage, SIGNAL(completeChanged()));
    connect(ui->codecCombo, SIGNAL(currentTextChanged(QString)), ui->dsPage, SIGNAL(completeChanged()));
    connect(ui->inputFileEdit, SIGNAL(textChanged(QString)), ui->dsPage, SIGNAL(completeChanged()));

    ui->dsTypeCombo->addItems(IMPORT_MANAGER->getImportDataSourceTypes());
}

void ImportDialog::removeOldOptions()
{
    pluginConfigOk.clear();
    safe_delete(configMapper);
    safe_delete(pluginOptionsWidget);
}

void ImportDialog::updateStandardOptions()
{
    bool showFileName = currentPlugin->standardOptionsToEnable().testFlag(ImportManager::FILE_NAME);
    bool showCodec = currentPlugin->standardOptionsToEnable().testFlag(ImportManager::CODEC);

    int row = 0;
    QGridLayout* grid = dynamic_cast<QGridLayout*>(ui->dsOptionsGroup->layout());
    if (showFileName)
    {
        grid->addWidget(ui->inputFileLabel, row, 0);
        grid->addWidget(ui->inputFileWidget, row, 1);
        row++;
    }
    else
    {
        grid->removeWidget(ui->inputFileLabel);
        grid->removeWidget(ui->inputFileWidget);
    }

    ui->inputFileLabel->setVisible(showFileName);
    ui->inputFileWidget->setVisible(showFileName);

    if (showCodec)
    {
        grid->addWidget(ui->codecLabel, row, 0);
        grid->addWidget(ui->codecCombo, row, 1);
        row++;
    }
    else
    {
        grid->removeWidget(ui->codecLabel);
        grid->removeWidget(ui->codecCombo);
    }

    ui->codecLabel->setVisible(showCodec);
    ui->codecCombo->setVisible(showCodec);
}

void ImportDialog::updatePluginOptions(int& rows)
{
    QString formName = currentPlugin->getImportConfigFormName();
    CfgMain* cfgMain = currentPlugin->getConfig();
    ui->dsPluginOptionsGroup->setVisible(false);
    if (formName.isNull() || !cfgMain)
    {
        if (!formName.isNull())
        {
            qWarning() << "FormName is given, but cfgMain is null in ImportDialog::updatePluginOptions() for plugin:" << currentPlugin->getName()
                       << ", formName:" << formName;
        }
        return;
    }

    if (!FORMS->hasWidget(formName))
    {
        qWarning() << "Import plugin" << currentPlugin->getName() << "requested for form named" << formName << "but FormManager doesn't have it."
                   << "Available forms are:" << FORMS->getAvailableForms();
        return;
    }

    pluginOptionsWidget = FORMS->createWidget(formName);
    if (!pluginOptionsWidget)
    {
        qWarning() << "Import plugin" << currentPlugin->getName() << "requested for form named" << formName << "but FormManager returned null.";
        return;
    }

    ui->dsPluginOptionsGroup->setVisible(true);

    if (pluginOptionsWidget->layout())
        pluginOptionsWidget->layout()->setContentsMargins(0, 0, 0, 0);

    ui->dsPluginOptionsGroup->layout()->addWidget(pluginOptionsWidget);
    rows++;

    configMapper = new ConfigMapper(cfgMain);
    configMapper->bindToConfig(pluginOptionsWidget);
    connect(configMapper, SIGNAL(modified(QWidget*)), this, SLOT(updateValidation()));
    updateValidation();
}

void ImportDialog::handleValidationResultFromPlugin(bool valid, CfgEntry* key, const QString& errorMsg)
{
    QWidget* w = configMapper->getBindWidgetForConfig(key);
    if (w)
        setValidState(w, valid, errorMsg);

    if (valid == pluginConfigOk.contains(key)) // if state changed
    {
        if (!valid)
            pluginConfigOk[key] = false;
        else
            pluginConfigOk.remove(key);
    }
}

void ImportDialog::stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled)
{
    QWidget* w = configMapper->getBindWidgetForConfig(key);
    if (!w)
        return;

    w->setVisible(visible);
    w->setEnabled(enabled);
}

void ImportDialog::refreshTables()
{
    Db* db = DBLIST->getByName(ui->dbNameCombo->currentText());
    if (db)
        tablesModel->setDb(db);
}

void ImportDialog::pluginSelected()
{
    ui->dsPluginOptionsGroup->setVisible(false);
    removeOldOptions();
    currentPlugin = IMPORT_MANAGER->getPluginForDataSourceType(ui->dsTypeCombo->currentText());
    if (!currentPlugin)
        return;

    updateStandardOptions();

    int rows = 0;
    updatePluginOptions(rows);
    ui->dsPluginOptionsGroup->setVisible(rows > 0);
}

void ImportDialog::updateValidation()
{
    if (!currentPlugin)
        return;

    currentPlugin->validateOptions();
    emit dsPageCompleteChanged();
}

void ImportDialog::pageChanged()
{
    if (currentPage() == ui->dsPage)
    {
        readStdConfig();
        updateValidation();
    }
}

void ImportDialog::browseForInputFile()
{
    if (!currentPlugin)
    {
        qCritical() << "Called ImportDialog::browseForInputFile(), but no ImportPlugin is selected.";
        return;
    }

    QString dir = getFileDialogInitPath();
    QString filter = currentPlugin->getFileFilter();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Pick file to import from"), dir, filter);
    if (fileName.isNull())
        return;

    ui->inputFileEdit->setText(fileName);
    setFileDialogInitPathByFile(fileName);
}

void ImportDialog::success()
{
    QWizard::accept();
}

void ImportDialog::hideCoverWidget()
{
    widgetCover->hide();
}

void ImportDialog::accept()
{
    if (!currentPlugin)
    {
        qCritical() << "Called ImportDialog::accept(), but no ImportPlugin is selected.";
        return;
    }

    ImportManager::StandardImportConfig stdConfig;
    if (currentPlugin->standardOptionsToEnable().testFlag(ImportManager::FILE_NAME))
        stdConfig.inputFileName = ui->inputFileEdit->text();

    if (currentPlugin->standardOptionsToEnable().testFlag(ImportManager::CODEC))
        stdConfig.codec = ui->codecCombo->currentText();

    stdConfig.ignoreErrors = ui->ignoreErrorsCheck->isChecked();

    storeStdConfig(stdConfig);
    configMapper->saveFromWidget(pluginOptionsWidget);

    Db* db = DBLIST->getByName(ui->dbNameCombo->currentText());;
    if (!db)
    {
        qCritical() << "Called ImportDialog::accept(), but no database is selected.";
        return;
    }

    QString table = ui->tableNameCombo->currentText();

    widgetCover->show();
    IMPORT_MANAGER->configure(currentPlugin->getDataSourceTypeName(), stdConfig);
    IMPORT_MANAGER->importToTable(db, table);
}

void ImportDialog::showEvent(QShowEvent* e)
{
    QWizard::showEvent(e);
    ui->tableNameCombo->setFocus();
}

void ImportDialog::keyPressEvent(QKeyEvent* e)
{
    if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) && QApplication::focusWidget() == ui->tableNameCombo)
    {
        next();
        return;
    }
    QWizard::keyPressEvent(e);
}
