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
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QTextCodec>
#include <QUiLoader>

ExportDialog::ExportDialog(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::ExportDialog)
{
    init();
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::init()
{
    ui->setupUi(this);
    initPageOrder();

    initModePage();
    initTablePage();
    initFormatPage();
    initQueryPage();
    initDbObjectsPage();

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
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
    exportMode = ExportManager::RESULTS;
    this->db = db;
    this->query = query;

    ui->queryDatabaseCombo->addItem(db->getName());
    ui->queryDatabaseCombo->setCurrentText(db->getName());
    ui->queryDatabaseCombo->setEnabled(false);
    ui->queryEdit->setPlainText(query);
    ui->queryEdit->setReadOnly(true);
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
        return ui->exportTableNameCombo->currentIndex() > -1;
    });

    dbListModel = new DbListModel(this);
    dbListModel->setCombo(ui->exportTableDbNameCombo);
    dbListModel->setSortMode(DbListModel::SortMode::Alphabetical);

    tablesModel = new DbObjListModel(this);
    tablesModel->setType(DbObjListModel::ObjectType::TABLE);
}

void ExportDialog::initQueryPage()
{
    ui->queryPage->setValidator([=]() -> bool
    {
        return ui->queryDatabaseCombo->currentIndex() > -1 && !ui->queryEdit->toPlainText().trimmed().isEmpty();
    });

    connect(ui->queryEdit, SIGNAL(textChanged()), ui->queryPage, SIGNAL(completeChanged()));
}

void ExportDialog::initDbObjectsPage()
{
    selectableDbListModel = new SelectableDbObjModel(this);
    selectableDbListModel->setSourceModel(DBTREE->getModel());
    ui->dbObjectsTree->setModel(selectableDbListModel);

    ui->databaseObjectsPage->setValidator([=]() -> bool
    {
        return selectableDbListModel->getCheckedObjects().size() > 0;
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
        if (ui->exportFileRadio->isChecked())
        {
            QString path = ui->exportFileEdit->text();
            if (path.trimmed().isEmpty())
                return false;

            QDir dir(path);
            if (dir.exists() && QFileInfo(path).isDir())
                return false;

            if (!dir.cdUp())
                return false;
        }
        return ui->formatCombo->currentIndex() > -1 && ui->encodingCombo->currentIndex() > -1;
    });

    ui->exportFileButton->setIcon(ICONS.EXPORT_FILE_BROWSE);
    connect(ui->exportFileButton, SIGNAL(clicked()), this, SLOT(browseForExportFile()));

    connect(ui->formatCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(pluginSelected()));
    connect(ui->formatCombo, SIGNAL(currentTextChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->encodingCombo, SIGNAL(currentTextChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportFileEdit, SIGNAL(textChanged(QString)), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportFileRadio, SIGNAL(clicked()), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
    connect(ui->exportClipboardRadio, SIGNAL(clicked()), ui->formatAndOptionsPage, SIGNAL(completeChanged()));
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

void ExportDialog::initPageOrder()
{
    setStartId(pageId(ui->exportSubjectPage));
    pageOrder[ExportManager::DATABASE] = {ui->databaseObjectsPage, ui->formatAndOptionsPage};
    pageOrder[ExportManager::TABLE] = {ui->tablePage, ui->formatAndOptionsPage};
    pageOrder[ExportManager::RESULTS] = {ui->queryPage, ui->formatAndOptionsPage};
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

        queryPageVisited = true;
    }
}

void ExportDialog::dbObjectsPageDisplayed()
{
    if (!dbObjectsPageVisited)
    {
        ui->dbObjectsDatabaseCombo->setModel(dbListModel);
        connect(ui->dbObjectsDatabaseCombo, SIGNAL(currentIndexChanged(int)), ui->queryPage, SIGNAL(completeChanged()));

        dbObjectsPageVisited = true;
    }
}

void ExportDialog::formatPageDisplayed()
{
    if (!formatPageVisited)
    {
        for (ExportPlugin* plugin : PLUGINS->getLoadedPlugins<ExportPlugin>())
            ui->formatCombo->addItem(plugin->getFormatName());

        ui->encodingCombo->addItems(textCodecNames());
        ui->encodingCombo->setCurrentText(defaultCodecName());

        formatPageVisited = true;
    }

//    qDebug() << selectableDbListModel->getCheckedObjects();
}

ExportPlugin* ExportDialog::getSelectedPlugin() const
{
    for (ExportPlugin* plugin : PLUGINS->getLoadedPlugins<ExportPlugin>())
    {
        if (plugin->getFormatName() == ui->formatCombo->currentText())
            return plugin;
    }
    return nullptr;
}

void ExportDialog::updateExportMode()
{
    if (ui->subjectDatabaseRadio->isChecked())
        exportMode = ExportManager::DATABASE;
    else if (ui->subjectTableRadio->isChecked())
        exportMode = ExportManager::TABLE;
    else if (ui->subjectQueryRadio->isChecked())
        exportMode = ExportManager::RESULTS;
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
    ui->optionsGroup->setVisible(false);

    ExportPlugin* plugin = getSelectedPlugin();
    if (!plugin)
    {
        qCritical() << "Could not find export plugin, while it was selected on ui:" << ui->formatCombo->currentText();
        return;
    }

    int optionsRow = 0;

    ExportManager::StandardConfigFlags options = plugin->standardOptionsToEnable();
    bool displayCodec = options.testFlag(ExportManager::CODEC);
    ui->encodingCombo->setVisible(displayCodec);
    ui->encodingLabel->setVisible(displayCodec);
    if (displayCodec)
        optionsRow++;

    updatePluginOptions(plugin, optionsRow);

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
}

void ExportDialog::dbObjectsSelectAll()
{
    selectableDbListModel->setRootChecked(true);
}

void ExportDialog::dbObjectsDeselectAll()
{
    selectableDbListModel->setRootChecked(false);
}

void ExportDialog::updatePluginOptions(ExportPlugin* plugin, int& optionsRow)
{
    safe_delete(pluginOptionsWidget);

    QString formName = plugin->getConfigFormName(exportMode);
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

    QGridLayout* grid = dynamic_cast<QGridLayout*>(ui->optionsGroup->layout());

    pluginOptionsWidget = FORMS->createWidget(formName);

    if (pluginOptionsWidget->layout())
        pluginOptionsWidget->layout()->setMargin(0);

    grid->addWidget(pluginOptionsWidget, 1, 0, 1, 2);
    optionsRow++;

    ConfigMapper mapper(cfgMain);
    mapper.loadToWidget(pluginOptionsWidget);
}
