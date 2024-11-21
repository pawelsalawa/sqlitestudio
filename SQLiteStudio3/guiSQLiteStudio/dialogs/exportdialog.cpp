#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "dblistmodel.h"
#include "dbobjlistmodel.h"
#include "services/dbmanager.h"
#include "uiutils.h"
#include "services/pluginmanager.h"
#include "formmanager.h"
#include "plugins/exportplugin.h"
#include "configmapper.h"
#include "selectabledbobjmodel.h"
#include "dbtree/dbtree.h"
#include "dbtree/dbtreemodel.h"
#include "schemaresolver.h"
#include "common/widgetcover.h"
#include "services/notifymanager.h"
#include "themetuner.h"
#include "uiconfig.h"
#include "common/dialogsizehandler.h"
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QUiLoader>
#include <QMimeData>

static const QString EXPORT_DIALOG_CFG_GROUP = "ExportDialog";
static const QString EXPORT_DIALOG_CFG_CODEC = "codec";
static const QString EXPORT_DIALOG_CFG_FILE = "outputFileName";
static const QString EXPORT_DIALOG_CFG_CLIP = "intoClipboard";
static const QString EXPORT_DIALOG_CFG_DATA = "exportData";
static const QString EXPORT_DIALOG_CFG_IDX = "exportTableIndexes";
static const QString EXPORT_DIALOG_CFG_TRIG = "exportTableTriggers";
static const QString EXPORT_DIALOG_CFG_FORMAT = "format";

ExportDialog::ExportDialog(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::ExportDialog)
{
    init();
}

ExportDialog::~ExportDialog()
{
    EXPORT_MANAGER->interrupt();
    safe_delete(configMapper);
    delete ui;
}

void ExportDialog::init()
{
    ui->setupUi(this);
    THEME_TUNER->darkThemeFix(this);
    limitDialogWidth(this);
    DialogSizeHandler::applyFor(this);

#ifdef Q_OS_MACX
    resize(width() + 150, height());
    setPixmap(QWizard::BackgroundPixmap, addOpacity(ICONS.DATABASE_EXPORT_WIZARD.toQIcon().pixmap(800, 800), 0.3));
#endif

    widgetCover = new WidgetCover(this);
    widgetCover->initWithInterruptContainer(tr("Cancel"));
    connect(widgetCover, SIGNAL(cancelClicked()), EXPORT_MANAGER, SLOT(interrupt()));
    connect(EXPORT_MANAGER, SIGNAL(finishedStep(int)), widgetCover, SLOT(setProgress(int)));
    widgetCover->setVisible(false);
    widgetCover->displayProgress(0, "%v");

    initPageOrder();

    initModePage();
    initTablePage();
    initFormatPage();
    initQueryPage();
    initDbObjectsPage();

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
    connect(EXPORT_MANAGER, SIGNAL(exportSuccessful()), this, SLOT(success()));
    connect(EXPORT_MANAGER, SIGNAL(exportFinished()), this, SLOT(hideCoverWidget()));
    connect(EXPORT_MANAGER, SIGNAL(storeInClipboard(QByteArray, QString)), this, SLOT(storeInClipboard(QByteArray, QString)));
    connect(EXPORT_MANAGER, SIGNAL(storeInClipboard(QString)), this, SLOT(storeInClipboard(QString)));
    connect(EXPORT_MANAGER, SIGNAL(validationResultFromPlugin(bool,CfgEntry*,QString)), this, SLOT(handleValidationResultFromPlugin(bool,CfgEntry*,QString)));
    connect(EXPORT_MANAGER, SIGNAL(stateUpdateRequestFromPlugin(CfgEntry*,bool,bool)), this, SLOT(stateUpdateRequestFromPlugin(CfgEntry*,bool,bool)));
}

void ExportDialog::setTableMode(Db* db, const QString& table)
{
    if (!db->isOpen())
    {
        qWarning() << "Cannot export from closed database.";
        return;
    }

    setStartId(pageId(ui->tablePage));
    exportMode = ExportManager::TABLE;
    this->db = db;
    this->table = table;

    ui->exportTableDbNameCombo->addItem(db->getName());
    ui->exportTableDbNameCombo->setCurrentText(db->getName());
    ui->exportTableDbNameCombo->setEnabled(false);
    ui->exportTableNameCombo->addItem(table);
    ui->exportTableNameCombo->setCurrentText(table);
    ui->exportTableNameCombo->setEnabled(false);
}

void ExportDialog::setQueryMode(Db* db, const QString& query)
{
    if (!db->isOpen())
    {
        qWarning() << "Cannot export from closed database.";
        return;
    }

    setStartId(pageId(ui->queryPage));
    exportMode = ExportManager::QUERY_RESULTS;
    this->db = db;
    this->query = query;

    ui->queryDatabaseCombo->addItem(db->getName());
    ui->queryDatabaseCombo->setCurrentText(db->getName());
    ui->queryDatabaseCombo->setEnabled(false);
    ui->queryEdit->setPlainText(query);
    updateQueryEditDb();
    ui->queryEdit->checkSyntaxNow();
}

void ExportDialog::setDatabaseMode(Db* db)
{
    if (!db->isOpen())
    {
        qWarning() << "Cannot export from closed database.";
        return;
    }

    setStartId(pageId(ui->databaseObjectsPage));
    exportMode = ExportManager::DATABASE;
    this->db = db;
}

void ExportDialog::setPreselectedDb(Db *db)
{
    if (!db->isOpen())
        return;

    this->db = db;
}

void ExportDialog::initModePage()
{
    connect(ui->subjectDatabaseRadio, SIGNAL(clicked()), this, SLOT(updateExportMode()));
    connect(ui->subjectTableRadio, SIGNAL(clicked()), this, SLOT(updateExportMode()));
    connect(ui->subjectQueryRadio, SIGNAL(clicked()), this, SLOT(updateExportMode()));
}

void ExportDialog::initTablePage()
{
    ui->tablePage->setValidator([=]() -> bool
    {
        bool dbOk = ui->exportTableDbNameCombo->currentIndex() > -1;
        bool tableOk = ui->exportTableNameCombo->currentIndex() > -1;

        setValidState(ui->exportTableDbNameCombo, dbOk, tr("Select database to export."));
        setValidState(ui->exportTableNameCombo, tableOk, tr("Select table to export."));

        return dbOk && tableOk;
    });

    dbListModel = new DbListModel(this);
    dbListModel->setCombo(ui->exportTableDbNameCombo);
    dbListModel->setSortMode(DbListModel::SortMode::AlphabeticalCaseInsensitive);

    tablesModel = new DbObjListModel(this);
    tablesModel->setType(DbObjListModel::ObjectType::TABLE);
    tablesModel->setSortMode(DbObjListModel::SortMode::AlphabeticalCaseInsensitive);

    connect(this, SIGNAL(tablePageCompleteChanged()), ui->tablePage, SIGNAL(completeChanged()));
}

void ExportDialog::initQueryPage()
{
    ui->queryPage->setValidator([=]() -> bool
    {
        bool queryOk = !ui->queryEdit->toPlainText().trimmed().isEmpty();
        queryOk &= ui->queryEdit->isSyntaxChecked() && !ui->queryEdit->haveErrors();
        bool dbOk = ui->queryDatabaseCombo->currentIndex() > -1;

        setValidState(ui->queryDatabaseCombo, dbOk, tr("Select database to export."));
        setValidState(ui->queryEdit, queryOk, tr("Enter valid query to export."));

        return dbOk && queryOk;
    });
    ui->queryEdit->setAlwaysEnforceErrorsChecking(true);

    connect(ui->queryEdit, SIGNAL(errorsChecked(bool)), ui->queryPage, SIGNAL(completeChanged()));
    connect(ui->queryEdit, SIGNAL(textChanged()), ui->queryPage, SIGNAL(completeChanged()));
    connect(ui->queryDatabaseCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateQueryEditDb()));
    connect(this, SIGNAL(queryPageCompleteChanged()), ui->queryPage, SIGNAL(completeChanged()));
}

void ExportDialog::initDbObjectsPage()
{
    selectableDbListModel = new SelectableDbObjModel(this);
    selectableDbListModel->setSourceModel(DBTREE->getModel());
    ui->dbObjectsTree->setModel(selectableDbListModel);

    ui->databaseObjectsPage->setValidator([=]() -> bool
    {
        bool dbOk = ui->dbObjectsDatabaseCombo->currentIndex() > -1;
        bool listOk = selectableDbListModel->getCheckedObjects().size() > 0;

        setValidState(ui->dbObjectsDatabaseCombo, dbOk, tr("Select database to export."));
        setValidState(ui->dbObjectsTree, listOk, tr("Select at least one object to export."));

        return listOk;
    });

    connect(ui->dbObjectsDatabaseCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateDbObjTree()));
    connect(ui->dbObjectsDatabaseCombo, SIGNAL(currentIndexChanged(QString)), ui->databaseObjectsPage, SIGNAL(completeChanged()));
    connect(selectableDbListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), ui->databaseObjectsPage, SIGNAL(completeChanged()));
    connect(ui->objectsSelectAllButton, SIGNAL(clicked()), this, SLOT(dbObjectsSelectAll()));
    connect(ui->objectsDeselectAllButton, SIGNAL(clicked()), this, SLOT(dbObjectsDeselectAll()));
}

void ExportDialog::initFormatPage()
{
    ui->formatAndOptionsPage->setValidator([=]() -> bool
    {
        setValidState(ui->exportFileEdit, true);
        bool outputFileSupported = currentPlugin && currentPlugin->getSupportedModes().testFlag(ExportManager::FILE);
        if (outputFileSupported && ui->exportFileRadio->isChecked())
        {
            QString path = ui->exportFileEdit->text();
            if (path.trimmed().isEmpty())
            {
                setValidState(ui->exportFileEdit, false, tr("You must provide a file name to export to."));
                return false;
            }

            QDir dir(path);
            if (dir.exists() && QFileInfo(path).isDir())
            {
                setValidState(ui->exportFileEdit, false, tr("Path you provided is an existing directory. You cannot overwrite it."));
                return false;
            }

            if (!dir.cdUp())
            {
                setValidState(ui->exportFileEdit, false, tr("The directory '%1' does not exist.").arg(dir.dirName()));
                return false;
            }

            QFileInfo fi(path);
            if (fi.exists())
                setValidStateInfo(ui->exportFileEdit, tr("The file '%1' exists and will be overwritten.").arg(fi.fileName()));
        }
        return ui->formatCombo->currentIndex() > -1 && ui->encodingCombo->currentIndex() > -1 && isPluginConfigValid();
    });

    ui->exportFileButton->setIcon(ICONS.EXPORT_FILE_BROWSE);
    connect(ui->exportFileButton, SIGNAL(clicked()), this, SLOT(browseForExportFile()));

    connect(ui->formatCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(pluginSelected()));
    connect(ui->formatCombo, SIGNAL(currentTextChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->encodingCombo, SIGNAL(currentTextChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportFileEdit, SIGNAL(textChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportFileRadio, SIGNAL(clicked()), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportClipboardRadio, SIGNAL(clicked()), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(this, SIGNAL(formatPageCompleteChanged()), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportFileRadio, SIGNAL(clicked()), this, SLOT(updateOptions()));
    connect(ui->exportClipboardRadio, SIGNAL(clicked()), this, SLOT(updateOptions()));
    connect(ui->exportFileRadio, SIGNAL(clicked()), this, SLOT(updateExportOutputOptions()));
    connect(ui->exportClipboardRadio, SIGNAL(clicked()), this, SLOT(updateExportOutputOptions()));
}

int ExportDialog::nextId() const
{
    if (exportMode == ExportManager::UNDEFINED)
        return pageId(ui->proxyPage);

    QList<QWizardPage*> order = pageOrder[exportMode];

    int idx = order.indexOf(currentPage());
    idx++;
    if (idx < order.size())
        return pageId(order[idx]);

    return -1;
}

bool ExportDialog::isPluginConfigValid() const
{
    return pluginConfigOk.size() == 0;
}

void ExportDialog::initPageOrder()
{
    setStartId(pageId(ui->exportSubjectPage));
    pageOrder[ExportManager::DATABASE] = {ui->databaseObjectsPage, ui->formatAndOptionsPage};
    pageOrder[ExportManager::TABLE] = {ui->tablePage, ui->formatAndOptionsPage};
    pageOrder[ExportManager::QUERY_RESULTS] = {ui->queryPage, ui->formatAndOptionsPage};
    updateExportMode();
}

int ExportDialog::pageId(QWizardPage* wizardPage) const
{
    for (int id : pageIds())
    {
        if (page(id) == wizardPage)
            return id;
    }
    return -1;
}

void ExportDialog::tablePageDisplayed()
{
    if (!tablePageVisited)
    {
        if (table.isNull()) // table mode selected by user, not forced by setTableMode().
        {
            ui->exportTableDbNameCombo->setModel(dbListModel);
            if (db)
                ui->exportTableDbNameCombo->setCurrentText(db->getName());

            connect(ui->exportTableDbNameCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDbTables()));

            ui->exportTableNameCombo->setModel(tablesModel);
            connect(ui->exportTableNameCombo, SIGNAL(currentTextChanged(QString)), ui->tablePage, SIGNAL(completeChanged()));
        }
        updateDbTables();
        emit tablePageCompleteChanged();
        tablePageVisited = true;
    }
}

void ExportDialog::queryPageDisplayed()
{
    if (!queryPageVisited)
    {
        if (query.isNull()) // query mode selected by user, not forced by setQueryMode().
        {
            ui->queryDatabaseCombo->setModel(dbListModel);
            if (db)
                ui->queryDatabaseCombo->setCurrentText(db->getName());

            connect(ui->queryDatabaseCombo, SIGNAL(currentIndexChanged(int)), ui->queryPage, SIGNAL(completeChanged()));
        }

        updateQueryEditDb();
        emit queryPageCompleteChanged();
        queryPageVisited = true;
    }
}

void ExportDialog::dbObjectsPageDisplayed()
{
    if (!dbObjectsPageVisited)
    {
        ui->dbObjectsDatabaseCombo->setModel(dbListModel);
        connect(ui->dbObjectsDatabaseCombo, SIGNAL(currentIndexChanged(int)), ui->queryPage, SIGNAL(completeChanged()));

        if (db)
            ui->dbObjectsDatabaseCombo->setCurrentText(db->getName());

        dbObjectsPageVisited = true;
    }
}

void ExportDialog::formatPageDisplayed()
{
    if (!formatPageVisited)
    {
        ui->formatCombo->addItems(EXPORT_MANAGER->getAvailableFormats(exportMode));

        ui->encodingCombo->addItems(textCodecNames());
        ui->encodingCombo->setCurrentText(defaultCodecName());

        formatPageVisited = true;
    }
    readStdConfigForLastPage();
    pluginSelected();

    emit formatPageCompleteChanged();
}

ExportPlugin* ExportDialog::getSelectedPlugin() const
{
    return EXPORT_MANAGER->getPluginForFormat(ui->formatCombo->currentText());
}

void ExportDialog::updateExportMode()
{
    if (ui->subjectDatabaseRadio->isChecked())
        exportMode = ExportManager::DATABASE;
    else if (ui->subjectTableRadio->isChecked())
        exportMode = ExportManager::TABLE;
    else if (ui->subjectQueryRadio->isChecked())
        exportMode = ExportManager::QUERY_RESULTS;
    else
        exportMode = ExportManager::UNDEFINED;
}

void ExportDialog::pageChanged(int pageId)
{
    QWizardPage* wizardPage = page(pageId);
    if (wizardPage == ui->tablePage)
        tablePageDisplayed();
    else if (wizardPage == ui->queryPage)
        queryPageDisplayed();
    else if (wizardPage == ui->databaseObjectsPage)
        dbObjectsPageDisplayed();
    else if (wizardPage == ui->formatAndOptionsPage)
        formatPageDisplayed();
    else if (wizardPage == ui->proxyPage)
        next();
}

void ExportDialog::updateDbTables()
{
    if (!table.isNull())
        return; // we don't want tables to be automatically updated if this is strictly set table

    QString dbName = ui->exportTableDbNameCombo->currentText();
    db = DBLIST->getByName(dbName);

    tablesModel->setDb(db);
}

void ExportDialog::browseForExportFile()
{
    QStringList filters;
    if (currentPlugin)
        filters << currentPlugin->getFormatName()+" (*." + currentPlugin->defaultFileExtension() + ")";

    filters << tr("All files (*)");

    QString dir = getFileDialogInitPath();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Pick file to export to"), dir, filters.join(";;"), 0, QFileDialog::DontConfirmOverwrite);
    if (fileName.isNull())
        return;

    if (currentPlugin && !fileName.endsWith("." + currentPlugin->defaultFileExtension()))
        fileName += "." + currentPlugin->defaultFileExtension();

    ui->exportFileEdit->setText(fileName);
    setFileDialogInitPathByFile(fileName);
}

void ExportDialog::pluginSelected()
{
    pluginConfigOk.clear();

    currentPlugin = getSelectedPlugin();
    if (!currentPlugin)
    {
        qCritical() << "Could not find export plugin, while it was selected on ui:" << ui->formatCombo->currentText();
        return;
    }

    currentPlugin->setExportMode(exportMode);

    updateExportOutputOptions();
    updateOptions();
}

void ExportDialog::updateExportOutputOptions()
{
    ExportManager::StandardConfigFlags options = currentPlugin->standardOptionsToEnable();
    bool displayCodec = options.testFlag(ExportManager::CODEC) && !ui->exportClipboardRadio->isChecked();
    bool clipboardSupported = currentPlugin->getSupportedModes().testFlag(ExportManager::CLIPBOARD);
    bool outputFileSupported = currentPlugin->getSupportedModes().testFlag(ExportManager::FILE);

    bool enabled = outputFileSupported && ui->exportFileRadio->isChecked();
    ui->exportFileEdit->setEnabled(enabled);
    ui->exportFileButton->setEnabled(enabled);

    ui->exportClipboardRadio->setVisible(clipboardSupported);
    ui->exportFileRadio->setVisible(outputFileSupported);
    ui->exportFileEdit->setVisible(outputFileSupported);
    ui->exportFileButton->setVisible(outputFileSupported);
    if (!clipboardSupported && outputFileSupported)
        ui->exportFileRadio->setChecked(true);

    ui->encodingCombo->setVisible(displayCodec);
    ui->encodingLabel->setVisible(displayCodec);
    if (displayCodec)
    {
        QString codec = CFG->get(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_CODEC).toString();
        QString defaultCodec = currentPlugin->getDefaultEncoding();
        if (codec.isNull())
            codec = defaultCodec;

        int idx = ui->encodingCombo->findText(codec);
        if (idx == -1 && codec != defaultCodec)
        {
            codec = defaultCodec;
            idx = ui->encodingCombo->findText(codec);
        }

        if (idx > -1)
            ui->encodingCombo->setCurrentIndex(idx);
    }

    ui->exportToGroup->setVisible(clipboardSupported || outputFileSupported || displayCodec);
}

void ExportDialog::updateQueryEditDb()
{
    Db* db = getDbForExport(ui->queryDatabaseCombo->currentText());
    ui->queryEdit->setDb(db);
}

void ExportDialog::updateOptions()
{
    ui->optionsGroup->setVisible(false);

    if (!currentPlugin)
    {
        qCritical() << "Could not find export plugin, while it was selected on ui:" << ui->formatCombo->currentText();
        return;
    }

    int optionsRow = 0;
    updatePluginOptions(currentPlugin, optionsRow);
    ui->optionsGroup->setVisible(optionsRow > 0);
}

void ExportDialog::updateDbObjTree()
{
    selectableDbListModel->setDbName(ui->dbObjectsDatabaseCombo->currentText());

    QModelIndex root = selectableDbListModel->index(0, 0);
    if (root.isValid())
    {
        root = setupNewDbObjTreeRoot(root);
        ui->dbObjectsTree->setRootIndex(root);

        ui->dbObjectsTree->expand(root);
        QModelIndex child;
        for (int i = 0; (child = selectableDbListModel->index(i, 0, root)).isValid(); i++)
            ui->dbObjectsTree->expand(child);
    }
    dbObjectsSelectAll();
}

void ExportDialog::dbObjectsSelectAll()
{
    selectableDbListModel->setRootChecked(true);
}

void ExportDialog::dbObjectsDeselectAll()
{
    selectableDbListModel->setRootChecked(false);
}

void ExportDialog::hideCoverWidget()
{
    widgetCover->hide();
}

void ExportDialog::storeInClipboard(const QByteArray& bytes, const QString& mimeType)
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, bytes);
    QApplication::clipboard()->setMimeData(mimeData);
}

void ExportDialog::storeInClipboard(const QString& str)
{
    QApplication::clipboard()->setText(str);
}

void ExportDialog::success()
{
    QWizard::accept();
}

void ExportDialog::accept()
{
    doExport();
}

int ExportDialog::exec()
{
    readStdConfigForFirstPage();
    return QDialog::exec();
}

void ExportDialog::updatePluginOptions(ExportPlugin* plugin, int& optionsRow)
{
    safe_delete(pluginOptionsWidget);

    QString formName = plugin->getExportConfigFormName();
    CfgMain* cfgMain = plugin->getConfig();
    if (formName.isNull() || !cfgMain)
    {
        if (!formName.isNull())
        {
            qWarning() << "FormName is given, but cfgMain is null in ExportDialog::updatePluginOptions() for plugin:" << plugin->getName()
                       << ", formName:" << formName;
        }
        return;
    }

    if (!FORMS->hasWidget(formName))
    {
        qWarning() << "Export plugin" << plugin->getName() << "requested for form named" << formName << "but FormManager doesn't have it."
                   << "Available forms are:" << FORMS->getAvailableForms();
        return;
    }

    safe_delete(configMapper);

    QGridLayout* grid = dynamic_cast<QGridLayout*>(ui->optionsGroup->layout());

    pluginOptionsWidget = FORMS->createWidget(formName);

    if (pluginOptionsWidget->layout())
        pluginOptionsWidget->layout()->setContentsMargins(0, 0, 0, 0);

    grid->addWidget(pluginOptionsWidget, 1, 0, 1, 2);
    optionsRow++;

    configMapper = new ConfigMapper(cfgMain);
    configMapper->bindToConfig(pluginOptionsWidget);
    connect(configMapper, SIGNAL(modified(QWidget*)), this, SLOT(updateValidation()));
    plugin->validateOptions();
}

void ExportDialog::storeStdConfig(const ExportManager::StandardExportConfig &stdConfig)
{
    CFG->begin();
    CFG->set(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_CODEC, stdConfig.codec);
    CFG->set(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_FILE, stdConfig.outputFileName);
    CFG->set(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_CLIP, stdConfig.intoClipboard);
    CFG->set(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_DATA, stdConfig.exportData);
    CFG->set(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_IDX, stdConfig.exportTableIndexes);
    CFG->set(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_TRIG, stdConfig.exportTableTriggers);
    CFG->set(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_FORMAT, currentPlugin->getFormatName());
    CFG->commit();
}

void ExportDialog::readStdConfigForFirstPage()
{
    bool exportData = CFG->get(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_DATA, true).toBool();
    if (exportMode == ExportManager::DATABASE)
        ui->exportDbDataCheck->setChecked(exportData);
    else if (exportMode == ExportManager::TABLE)
        ui->exportTableDataCheck->setChecked(exportData);

    ui->exportTableIndexesCheck->setChecked(CFG->get(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_IDX, true).toBool());
    ui->exportTableTriggersCheck->setChecked(CFG->get(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_TRIG, true).toBool());
}

void ExportDialog::readStdConfigForLastPage()
{
    QString format = CFG->get(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_FORMAT).toString();
    int idx = ui->formatCombo->findText(format);
    if (idx > -1)
        ui->formatCombo->setCurrentIndex(idx);

    bool useClipboard = CFG->get(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_CLIP, false).toBool();
    ui->exportFileRadio->setChecked(!useClipboard);
    ui->exportClipboardRadio->setChecked(useClipboard);
    ui->exportFileEdit->setText(CFG->get(EXPORT_DIALOG_CFG_GROUP, EXPORT_DIALOG_CFG_FILE, QString()).toString());

    // Codec is read within updateExportOutputOptions()
}

void ExportDialog::updateValidation()
{
    if (!currentPlugin)
        return;

    currentPlugin->validateOptions();
    emit formatPageCompleteChanged();
}

void ExportDialog::doExport()
{
    widgetCover->show();

    ExportManager::StandardExportConfig stdConfig = getExportConfig();
    storeStdConfig(stdConfig);
    configMapper->saveFromWidget(pluginOptionsWidget);

    QString format = ui->formatCombo->currentText();
    switch (exportMode)
    {
        case ExportManager::DATABASE:
            exportDatabase(stdConfig, format);
            break;
        case ExportManager::TABLE:
            exportTable(stdConfig, format);
            break;
        case ExportManager::QUERY_RESULTS:
            exportQuery(stdConfig, format);
            break;
        case ExportManager::UNDEFINED:
            qCritical() << "Finished export dialog with undefined mode.";
            notifyInternalError();
            break;
        case ExportManager::FILE:
        case ExportManager::CLIPBOARD:
            break;
    }
}

void ExportDialog::exportDatabase(const ExportManager::StandardExportConfig& stdConfig, const QString& format)
{
    Db* db = getDbForExport(ui->dbObjectsDatabaseCombo->currentText());
    if (!db || !db->isValid())
        return;

    EXPORT_MANAGER->configure(format, stdConfig);
    EXPORT_MANAGER->exportDatabase(db, selectableDbListModel->getCheckedObjects());
}

void ExportDialog::exportTable(const ExportManager::StandardExportConfig& stdConfig, const QString& format)
{
    Db* db = getDbForExport(ui->exportTableDbNameCombo->currentText());
    if (!db || !db->isValid())
        return;

    EXPORT_MANAGER->configure(format, stdConfig);
    // TODO when dbnames are fully supported, pass the dbname below
    EXPORT_MANAGER->exportTable(db, QString(), ui->exportTableNameCombo->currentText());
}

void ExportDialog::exportQuery(const ExportManager::StandardExportConfig& stdConfig, const QString& format)
{
    Db* db = getDbForExport(ui->queryDatabaseCombo->currentText());
    if (!db || !db->isValid())
        return;

    EXPORT_MANAGER->configure(format, stdConfig);
    EXPORT_MANAGER->exportQueryResults(db, ui->queryEdit->toPlainText());
}

ExportManager::StandardExportConfig ExportDialog::getExportConfig() const
{
    bool clipboardSupported = currentPlugin->getSupportedModes().testFlag(ExportManager::CLIPBOARD);
    bool outputFileSupported = currentPlugin->getSupportedModes().testFlag(ExportManager::FILE);
    bool clipboard = clipboardSupported && ui->exportClipboardRadio->isChecked();

    ExportManager::StandardExportConfig stdConfig;
    stdConfig.intoClipboard = clipboard;

    if (clipboard)
        stdConfig.outputFileName = QString();
    else if (outputFileSupported)
        stdConfig.outputFileName = ui->exportFileEdit->text();

    if (exportMode == ExportManager::DATABASE)
        stdConfig.exportData = ui->exportDbDataCheck->isChecked();
    else if (exportMode == ExportManager::TABLE)
        stdConfig.exportData = ui->exportTableDataCheck->isChecked();
    else
        stdConfig.exportData = false;

    stdConfig.exportTableIndexes = ui->exportTableIndexesCheck->isChecked();
    stdConfig.exportTableTriggers = ui->exportTableTriggersCheck->isChecked();

    if (ui->encodingCombo->isVisible() && ui->encodingCombo->currentIndex() > -1)
        stdConfig.codec = ui->encodingCombo->currentText();
    else
        stdConfig.codec = defaultCodecName();

    return stdConfig;
}

Db* ExportDialog::getDbForExport(const QString& name)
{
    Db* db = DBLIST->getByName(name);
    if (!db)
    {
        qCritical() << "Could not find db selected in combo:" << name;
        notifyInternalError();
        return nullptr;
    }
    return db;
}

void ExportDialog::notifyInternalError()
{
    notifyError(tr("Internal error during export. This is a bug. Please report it."));
}

QModelIndex ExportDialog::setupNewDbObjTreeRoot(const QModelIndex& root)
{
    QModelIndex newRoot = root;
    DbTreeItem* item = nullptr;
    while (newRoot.isValid())
    {
        item = selectableDbListModel->getItemForIndex(newRoot);
        if (item->getType() == DbTreeItem::Type::DB)
            return newRoot;

        newRoot = selectableDbListModel->index(0, 0, newRoot);
    }
    return newRoot;
}

void ExportDialog::handleValidationResultFromPlugin(bool valid, CfgEntry* key, const QString& errorMsg)
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

        emit formatPageCompleteChanged();
    }
}

void ExportDialog::stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled)
{
    QWidget* w = configMapper->getBindWidgetForConfig(key);
    if (!w)
        return;

    w->setVisible(visible);
    w->setEnabled(enabled);
}
