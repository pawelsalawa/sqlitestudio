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

void PopulateDialog::init()
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Populate", "populate dialog button"));

    plugins = PLUGINS->getLoadedPlugins<PopulatePlugin>();
    qSort(plugins.begin(), plugins.end(), [](PopulatePlugin* p1, PopulatePlugin* p2) -> bool
    {
        return p1->getTitle().compare(p2->getTitle()) < 0;
    });

    for (PopulatePlugin* plugin : plugins)
        pluginTitles << plugin->getTitle();

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

void PopulateDialog::refreshTables()
{
    db = DBLIST->getByName(ui->databaseCombo->currentText());
    if (db)
        tablesModel->setDb(db);
}

void PopulateDialog::refreshColumns()
{
    for (const ColumnEntry& entry : columnEntries)
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

    buttonMapper = new QSignalMapper(this);
    connect(buttonMapper, SIGNAL(mapped(int)), this, SLOT(configurePlugin(int)));

    checkMapper = new QSignalMapper(this);
    connect(checkMapper, SIGNAL(mapped(int)), this, SLOT(updateColumnState(int)));

    SchemaResolver resolver(db);
    QStringList columns = resolver.getTableColumns(ui->tableCombo->currentText());
    QCheckBox* check;
    QComboBox* combo;
    QToolButton* btn;
    int row = 0;
    for (const QString& column : columns)
    {
        check = new QCheckBox(column);
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
        columnEntries << ColumnEntry(check, combo, btn);
        row++;
    }

    row = 0;
    for (const ColumnEntry& entry : columnEntries)
    {
        pluginSelected(entry.combo, entry.combo->currentIndex());
        updateColumnState(row++);
    }

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    ui->columnsLayout->addItem(spacer, row, 0, 1, 3);
}

void PopulateDialog::pluginSelected(int index)
{
    QComboBox* cb = dynamic_cast<QComboBox*>(sender());
    pluginSelected(cb, index);
}

void PopulateDialog::pluginSelected(QComboBox* combo, int index)
{
    if (!combo)
        return;

    ColumnEntry* entry = nullptr;

    for (ColumnEntry& e : columnEntries)
    {
        if (e.combo == combo)
        {
            entry  = &e;
            break;
        }
    }

    if (!entry)
        return;

    safe_delete(entry->engine);

    if (index < 0 || index >= plugins.size())
        return;

    entry->engine = plugins[index]->createEngine();
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

    PopulateConfigDialog dialog(engine, columnEntries[index].check->text(), columnEntries[index].combo->currentText(), this);
    if (dialog.exec() != QDialog::Accepted)
        engine->getConfig()->restore();

    engine->getConfig()->release();
}

void PopulateDialog::updateColumnState(int index)
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
}

void PopulateDialog::accept()
{
    if (!db)
        return;

    QHash<QString,PopulateEngine*> engines;
    for (ColumnEntry& entry : columnEntries)
    {
        if (!entry.check->isChecked())
            continue;

        if (!entry.engine)
            return;

        engines[entry.check->text()] = entry.engine;
        entry.engine = nullptr; // to avoid deleting it in the entry's destructor - worker will delete it after it's done
    }

    QString table = ui->tableCombo->currentText();
    qint64 rows = ui->rowsSpin->value();
    POPULATE_MANAGER->populate(db, table, engines, rows);
}

PopulateDialog::ColumnEntry::ColumnEntry(QCheckBox* check, QComboBox* combo, QToolButton* button) :
    check(check), combo(combo), button(button)
{
}

PopulateDialog::ColumnEntry::~ColumnEntry()
{
    safe_delete(engine);
}
