#include "populatedialog.h"
#include "ui_populatedialog.h"
#include "dblistmodel.h"
#include "dbobjlistmodel.h"
#include "services/dbmanager.h"
#include "schemaresolver.h"
#include "services/pluginmanager.h"
#include "plugins/populateplugin.h"
#include "populateconfigdialog.h"
#include "uiutils.h"
#include "services/populatemanager.h"
#include "common/widgetcover.h"
#include "common/compatibility.h"
#include "common/dialogsizehandler.h"
#include <QPushButton>
#include <QGridLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QDebug>
#include <QSignalMapper>

PopulateDialog::PopulateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PopulateDialog)
{
    init();
}

PopulateDialog::~PopulateDialog()
{
    delete ui;
}

void PopulateDialog::setDbAndTable(Db* db, const QString& table)
{
    QString oldTable = ui->tableCombo->currentText();
    ui->databaseCombo->setCurrentText(db->getName());
    ui->tableCombo->setCurrentText(table);

    // #4177
    if (oldTable == table)
        refreshColumns();
}

void PopulateDialog::init()
{
    ui->setupUi(this);
    limitDialogWidth(this);
    DialogSizeHandler::applyFor(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Populate", "populate dialog button"));

    plugins = PLUGINS->getLoadedPlugins<PopulatePlugin>();
    sSort(plugins, [](PopulatePlugin* p1, PopulatePlugin* p2) -> bool
    {
        return p1->getTitle().compare(p2->getTitle()) < 0;
    });

    for (PopulatePlugin*& plugin : plugins)
    {
        pluginByName[plugin->getName()] = plugin;
        pluginTitles << plugin->getTitle();
    }

    widgetCover = new WidgetCover(this);
    widgetCover->initWithInterruptContainer(tr("Abort"));
    widgetCover->setVisible(false);
    connect(widgetCover, SIGNAL(cancelClicked()), POPULATE_MANAGER, SLOT(interrupt()));

    ui->scrollArea->setAutoFillBackground(false);
    ui->scrollArea->viewport()->setAutoFillBackground(false);
    ui->columnsWidget->setAutoFillBackground(false);

    dbListModel = new DbListModel(this);
    dbListModel->setCombo(ui->databaseCombo);
    dbListModel->setSortMode(DbListModel::SortMode::Alphabetical);
    ui->databaseCombo->setModel(dbListModel);

    tablesModel = new DbObjListModel(this);
    tablesModel->setIncludeSystemObjects(false);
    tablesModel->setType(DbObjListModel::ObjectType::TABLE);
    ui->tableCombo->setModel(tablesModel);
    refreshTables();

    connect(ui->databaseCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(refreshTables()));
    connect(ui->tableCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(refreshColumns()));
    connect(POPULATE_MANAGER, SIGNAL(populatingFinished()), widgetCover, SLOT(hide()));
    connect(POPULATE_MANAGER, SIGNAL(finishedStep(int)), widgetCover, SLOT(setProgress(int)));
    connect(POPULATE_MANAGER, SIGNAL(populatingSuccessful()), this, SLOT(finished()));
}

PopulateEngine* PopulateDialog::getEngine(int selectedPluginIndex)
{
    if (selectedPluginIndex < 0 || selectedPluginIndex >= plugins.size())
    {
        qCritical() << "Selected populate plugin out of range!";
        return nullptr;
    }

    return plugins[selectedPluginIndex]->createEngine();
}

void PopulateDialog::deleteEngines(const QList<PopulateEngine*>& engines)
{
    for (PopulateEngine* engine : engines)
        delete engine;
}

void PopulateDialog::rebuildEngines(const QHash<QString, QPair<QString, QVariant>>& columnConfig)
{
    int row = 0;
    QVariant config;
    QString pluginName;
    for (ColumnEntry& entry : columnEntries)
    {
        pluginName.clear();
        if (columnConfig.contains(entry.column))
            pluginName = columnConfig[entry.column].first;

        if (pluginName.isNull())
        {
            pluginName = plugins[entry.combo->currentIndex()]->getName();
            config = CFG->getPopulateHistory(pluginName);
        }
        else
        {
            entry.combo->setCurrentIndex(plugins.indexOf(pluginByName[pluginName]));
            config = columnConfig[entry.column].second;
        }

        pluginSelected(entry.combo, entry.combo->currentIndex(), config);
        updateColumnState(row++, false);
    }
}

void PopulateDialog::refreshTables()
{
    db = DBLIST->getByName(ui->databaseCombo->currentText());
    if (db)
        tablesModel->setDb(db);

    updateState();
}

void PopulateDialog::refreshColumns()
{
    for (ColumnEntry& entry : columnEntries)
    {
        delete entry.check;
        delete entry.combo;
        delete entry.button;
    }
    columnEntries.clear();
    safe_delete(buttonMapper);
    safe_delete(checkMapper);

    delete ui->columnsLayout;
    ui->columnsLayout = new QGridLayout();
    ui->columnsWidget->setLayout(ui->columnsLayout);

    if (!db)
    {
        qCritical() << "No Db while refreshing columns in PopulateDialog!";
        return;
    }

    QString table = ui->tableCombo->currentText();

    buttonMapper = new QSignalMapper(this);
    connect(buttonMapper, SIGNAL(mapped(int)), this, SLOT(configurePlugin(int)));

    checkMapper = new QSignalMapper(this);
    connect(checkMapper, SIGNAL(mapped(int)), this, SLOT(updateColumnState(int)));

    SchemaResolver resolver(db);
    QStringList columns = resolver.getTableColumns(table);
    QCheckBox* check = nullptr;
    QComboBox* combo = nullptr;
    QToolButton* btn = nullptr;

    int rows = -1;
    QHash<QString, QPair<QString, QVariant>> columnConfig = CFG->getPopulateHistory(db->getName(), table, rows);
    if (rows > -1)
        ui->rowsSpin->setValue(rows);

    int row = 0;
    for (const QString& column : columns)
    {
        check = new QCheckBox(column);
        if (columnConfig.contains(column))
            check->setChecked(true);

        connect(check, SIGNAL(toggled(bool)), checkMapper, SLOT(map()));
        checkMapper->setMapping(check, row);

        combo = new QComboBox();
        combo->addItems(pluginTitles);
        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(pluginSelected(int)));

        btn = new QToolButton();
        btn->setText(tr("Configure"));
        connect(btn, SIGNAL(clicked()), buttonMapper, SLOT(map()));
        buttonMapper->setMapping(btn, row);

        ui->columnsLayout->addWidget(check, row, 0);
        ui->columnsLayout->addWidget(combo, row, 1);
        ui->columnsLayout->addWidget(btn, row, 2);
        columnEntries << ColumnEntry(column, check, combo, btn);
        row++;
    }

    rebuildEngines(columnConfig);

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    ui->columnsLayout->addItem(spacer, row, 0, 1, 3);

    updateState();
}

void PopulateDialog::pluginSelected(int index)
{
    QVariant config;
    if (index >= 0 && index < plugins.size())
        config = CFG->getPopulateHistory(plugins[index]->getName());

    QComboBox* cb = dynamic_cast<QComboBox*>(sender());
    pluginSelected(cb, index, config);
}

void PopulateDialog::pluginSelected(QComboBox* combo, int index, const QVariant& config)
{
    if (!combo)
        return;

    ColumnEntry* entry = nullptr;

    int columnIndex = 0;
    for (ColumnEntry& e : columnEntries)
    {
        if (e.combo == combo)
        {
            entry  = &e;
            break;
        }
        columnIndex++;
    }

    if (!entry)
        return;

    safe_delete(entry->engine);

    if (index < 0 || index >= plugins.size())
        return;

    entry->plugin = plugins[index];
    entry->engine = entry->plugin->createEngine();
    if (config.isValid())
        entry->engine->getConfig()->setValuesFromQVariant(config);

    updateColumnState(columnIndex);
}

void PopulateDialog::configurePlugin(int index)
{
    if (index < 0 || index >= columnEntries.size())
    {
        qCritical() << "Plugin configure index out of range:" << index << "," << columnEntries.size();
        return;
    }

    PopulateEngine* engine = columnEntries[index].engine;
    if (!engine->getConfig())
    {
        qWarning() << "Called config on populate plugin, but it has no CfgMain.";
        return;
    }

    engine->getConfig()->savepoint();

    QString colName = columnEntries[index].column;
    PopulateConfigDialog dialog(engine, colName, columnEntries[index].combo->currentText(), this);
    if (dialog.exec() != QDialog::Accepted)
        engine->getConfig()->restore();

    engine->getConfig()->release();

    updateColumnState(index);
}

void PopulateDialog::updateColumnState(int index, bool updateGlobalState)
{
    if (index < 0 || index >= columnEntries.size())
    {
        qCritical() << "Column update called but index out of range:" << index << "," << columnEntries.size();
        return;
    }

    bool checked = columnEntries[index].check->isChecked();
    bool hasConfig = columnEntries[index].engine->getConfig() != nullptr;
    columnEntries[index].combo->setEnabled(checked);
    columnEntries[index].button->setEnabled(checked && hasConfig);

    bool valid = true;
    if (checked && hasConfig)
    {
        valid = columnEntries[index].engine->validateOptions();
        setValidState(columnEntries[index].button, valid, tr("Populating configuration for this column is invalid or incomplete."));
    }

    if (valid == columnsValid.contains(index)) // if state changed
    {
        if (!valid)
            columnsValid[index] = false;
        else
            columnsValid.remove(index);
    }

    if (updateGlobalState)
        updateState();
}

void PopulateDialog::updateState()
{
    bool columnsOk = columnsValid.size() == 0;
    bool dbOk = !ui->databaseCombo->currentText().isNull();
    bool tableOk = !ui->tableCombo->currentText().isNull();

    bool colCountOk = false;
    for (ColumnEntry& entry : columnEntries)
    {
        if (entry.check->isChecked())
        {
            colCountOk = true;
            break;
        }
    }

    setValidState(ui->databaseCombo, dbOk, tr("Select database with table to populate"));
    setValidState(ui->tableCombo, tableOk, tr("Select table to populate"));
    setValidState(ui->columnsGroup, (!tableOk || colCountOk), tr("You have to select at least one column."));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(columnsOk && tableOk && colCountOk);
}

void PopulateDialog::finished()
{
    QDialog::accept();
}

void PopulateDialog::accept()
{
    if (!db)
        return;

    QHash<QString, QPair<QString, QVariant>> configForHistory;
    QHash<QString,PopulateEngine*> engines;
    for (ColumnEntry& entry : columnEntries)
    {
        if (!entry.check->isChecked())
            continue;

        if (!entry.engine)
            return;

        engines[entry.column] = entry.engine;
        // entry.engine = nullptr; // to avoid deleting it in the entry's destructor - worker will delete it after it's done

        configForHistory[entry.column] = QPair<QString, QVariant>(entry.plugin->getName(), entry.engine->getConfig()->toQVariant());
    }

    QString table = ui->tableCombo->currentText();
    int rows = ui->rowsSpin->value();

    started = true;
    widgetCover->displayProgress(rows, "%v / %m");
    widgetCover->show();
    CFG->addPopulateHistory(db->getName(), table, rows, configForHistory);
    POPULATE_MANAGER->populate(db, table, engines, rows);
}

void PopulateDialog::reject()
{
    if (started)
        POPULATE_MANAGER->interrupt();

    QDialog::reject();
}

PopulateDialog::ColumnEntry::ColumnEntry(const QString& column, QCheckBox* check, QComboBox* combo, QToolButton* button) :
    column(column), check(check), combo(combo), button(button)
{
}

PopulateDialog::ColumnEntry::~ColumnEntry()
{
    safe_delete(engine);
}
