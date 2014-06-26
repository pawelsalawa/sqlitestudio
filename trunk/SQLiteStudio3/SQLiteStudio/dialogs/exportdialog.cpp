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
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QTextCodec>
#include <QUiLoader>
#include <QMimeData>

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

    widgetCover = new WidgetCover(this);
    widgetCover->setVisible(false);

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
    dbListModel->setSortMode(DbListModel::SortMode::Alphabetical);

    tablesModel = new DbObjListModel(this);
    tablesModel->setType(DbObjListModel::ObjectType::TABLE);

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
        if (ui->exportFileRadio->isChecked())
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
                setValidStateWarning(ui->exportFileEdit, tr("The file '%1' exists and will be overwritten.").arg(fi.fileName()));
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
    pluginSelected();
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
    QString fileName = QFileDialog::getSaveFileName(this, tr("Pick file to export to"), QString(), QString(), 0, QFileDialog::DontConfirmOverwrite);
    if (fileName.isNull())
        return;

    ui->exportFileEdit->setText(fileName);
}

void ExportDialog::pluginSelected()
{
    currentPlugin = getSelectedPlugin();
    if (!currentPlugin)
    {
        qCritical() << "Could not find export plugin, while it was selected on ui:" << ui->formatCombo->currentText();
        return;
    }

    currentPlugin->setExportMode(exportMode);
    bool clipboardSupported = currentPlugin->getSupportedModes().testFlag(ExportManager::CLIPBOARD);

    ui->exportClipboardRadio->setVisible(clipboardSupported);
    if (!clipboardSupported)
        ui->exportFileRadio->setChecked(true);

    updateExportOutputOptions();
    updateOptions();

    if (currentPlugin->getConfig() && !currentPlugin->getConfig()->isPersistable())
        currentPlugin->getConfig()->reset();
}

void ExportDialog::updateExportOutputOptions()
{
    bool enabled = ui->exportFileRadio->isChecked();
    ui->exportFileEdit->setEnabled(enabled);
    ui->exportFileButton->setEnabled(enabled);
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

    ExportManager::StandardConfigFlags options = currentPlugin->standardOptionsToEnable();
    bool displayCodec = options.testFlag(ExportManager::CODEC) && !ui->exportClipboardRadio->isChecked();
    ui->encodingCombo->setVisible(displayCodec);
    ui->encodingLabel->setVisible(displayCodec);

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
        ui->dbObjectsTree->expand(root);
        QModelIndex child;
        for (int i = 0; (child = root.child(i, 0)).isValid(); i++)
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
        pluginOptionsWidget->layout()->setMargin(0);

    grid->addWidget(pluginOptionsWidget, 1, 0, 1, 2);
    optionsRow++;

    configMapper = new ConfigMapper(cfgMain);
    configMapper->bindToConfig(pluginOptionsWidget);
    connect(configMapper, SIGNAL(modified()), this, SLOT(updateValidation()));
    plugin->validateOptions();
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
    EXPORT_MANAGER->exportTable(db, QString::null, ui->exportTableNameCombo->currentText());
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
    bool clipboard = ui->exportClipboardRadio->isChecked();

    ExportManager::StandardExportConfig stdConfig;
    stdConfig.intoClipboard = clipboard;

    if (clipboard)
        stdConfig.outputFileName = QString::null;
    else
        stdConfig.outputFileName = ui->exportFileEdit->text();

    if (exportMode == ExportManager::DATABASE)
        stdConfig.exportData = ui->exportDbDataCheck->isChecked();
    else if (exportMode == ExportManager::TABLE)
        stdConfig.exportData = ui->exportTableDataCheck->isChecked();
    else
        stdConfig.exportData = false;

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
