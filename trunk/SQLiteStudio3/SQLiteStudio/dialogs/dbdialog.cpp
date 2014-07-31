#include "dbdialog.h"
#include "ui_dbdialog.h"
#include "services/pluginmanager.h"
#include "plugins/dbplugin.h"
#include "uiutils.h"
#include "common/utils.h"
#include "services/dbmanager.h"
#include "common/global.h"
#include "iconmanager.h"
#include "common/unused.h"
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QDebug>
#include <QPushButton>
#include <QFileDialog>

DbDialog::DbDialog(Mode mode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DbDialog),
    mode(mode)
{
    init();
}

DbDialog::~DbDialog()
{
    delete ui;
}

void DbDialog::setDb(Db* db)
{
    this->db = db;
}

void DbDialog::setPermanent(bool perm)
{
    ui->permamentCheckBox->setChecked(perm);
}

QString DbDialog::getPath()
{
    return ui->fileEdit->text();
}

void DbDialog::setPath(const QString& path)
{
    ui->fileEdit->setText(path);
}

QString DbDialog::getName()
{
    return ui->nameEdit->text();
}

Db* DbDialog::getDb()
{
    Db* testDb = nullptr;
    QHash<QString, QVariant> options = collectOptions();
    QString path = ui->fileEdit->text();
    foreach (DbPlugin* plugin, dbPlugins)
    {
        testDb = plugin->getInstance("", path, options);
        if (testDb)
            return testDb;
    }
    return testDb;
}

bool DbDialog::isPermanent()
{
    return ui->permamentCheckBox->isChecked();
}

void DbDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void DbDialog::showEvent(QShowEvent *e)
{
    if (db)
    {
        int idx = ui->typeCombo->findText(db->getTypeLabel());
        ui->typeCombo->setCurrentIndex(idx);
        ui->typeCombo->setEnabled(false); // converting to other type is in separate dialog, it's different feature

        ui->generateCheckBox->setChecked(false);
        ui->fileEdit->setText(db->getPath());
        ui->nameEdit->setText(db->getName());
    }
    else if (ui->typeCombo->count() > 0)
    {
        int idx = ui->typeCombo->findText("SQLite3"); // we should have SQLite3 plugin
        if (idx > -1)
            ui->typeCombo->setCurrentIndex(idx);
        else
            ui->typeCombo->setCurrentIndex(0);
    }

    existingDatabaseNames = DBLIST->getDbNames();
    if (mode == EDIT)
        existingDatabaseNames.removeOne(db->getName());

    updateOptions();
    updateState();

    QDialog::showEvent(e);
}

void DbDialog::init()
{
    ui->setupUi(this);

    ui->browseButton->setIcon(ICONS.DATABASE_FILE);
    dbPlugins = PLUGINS->getLoadedPlugins<DbPlugin>();
    foreach (DbPlugin* dbPlugin, dbPlugins)
    {
        ui->typeCombo->addItem(dbPlugin->getLabel());
    }

    ui->browseButton->setVisible(true);
    ui->testConnIcon->setVisible(false);

    connect(ui->fileEdit, SIGNAL(textChanged(QString)), this, SLOT(fileChanged(QString)));
    connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(nameModified(QString)));
    connect(ui->generateCheckBox, SIGNAL(toggled(bool)), this, SLOT(generateNameSwitched(bool)));
    connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(browseClicked()));
    connect(ui->testConnButton, SIGNAL(clicked()), this, SLOT(testConnectionClicked()));
    connect(ui->typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(dbTypeChanged(int)));

    generateNameSwitched(true);
}

void DbDialog::updateOptions()
{
    setUpdatesEnabled(false);

    // Remove olds
    foreach (QWidget* w, optionWidgets)
    {
        ui->gridLayout->removeWidget(w);
        delete w;
    }
    adjustSize();

    optionWidgets.clear();
    optionKeyToWidget.clear();
    optionKeyToType.clear();
    helperToKey.clear();

    // Retrieve new list
    DbPlugin* plugin = nullptr;
    if (dbPlugins.count() > 0)
    {
        int idx = ui->typeCombo->currentIndex();
        if (idx > -1 )
        {
            plugin = dbPlugins[idx];
            QList<DbPluginOption> optList = plugin->getOptionsList();
            if (optList.size() > 0)
            {
                // Add new options
                int row = 3;
                foreach (DbPluginOption opt, optList)
                    addOption(opt, row++);
            }
        }
    }

    adjustSize();
    setUpdatesEnabled(true);
}

void DbDialog::addOption(const DbPluginOption& option, int row)
{
    QLabel* label = new QLabel(option.label, this);
    QWidget* editor = nullptr;
    QWidget* editorHelper = nullptr; // TODO, based on plugins for Url handlers

    editor = getEditor(option.type, editorHelper);
    Q_ASSERT(editor != nullptr);

    optionWidgets << label << editor;

    optionKeyToWidget[option.key] = editor;
    optionKeyToType[option.key] = option.type;
    ui->gridLayout->addWidget(label, row, 0);
    ui->gridLayout->addWidget(editor, row, 1);
    if (editorHelper)
    {
        ui->gridLayout->addWidget(editorHelper, row, 2);
        optionWidgets << editorHelper;
        helperToKey[editorHelper] = option.key;
    }
}

QWidget *DbDialog::getEditor(DbPluginOption::Type type, QWidget*& editorHelper)
{
    QWidget* editor = nullptr;
    editorHelper = nullptr;
    switch (type)
    {
        case DbPluginOption::STRING:
            editor = new QLineEdit(this);
            connect(editor, SIGNAL(textChanged(QString)), this, SLOT(propertyChanged()));
            break;
        case DbPluginOption::INT:
            editor = new QSpinBox(this);
            connect(editor, SIGNAL(valueChanged(int)), this, SLOT(propertyChanged()));
            break;
        case DbPluginOption::FILE:
            editor = new QLineEdit(this);
            editorHelper = new QPushButton(tr("Browse"), this);
            connect(editor, SIGNAL(textChanged(QString)), this, SLOT(propertyChanged()));
            connect(editorHelper, SIGNAL(pressed()), this, SLOT(browseForFile()));
            break;
        case DbPluginOption::BOOL:
            editor = new QCheckBox(this);
            connect(editor, SIGNAL(stateChanged(int)), this, SLOT(propertyChanged()));
            break;
        case DbPluginOption::DOUBLE:
            editor = new QDoubleSpinBox(this);
            connect(editor, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
            break;
        default:
            // TODO plugin based handling of custom editors
            break;
    }
    return editor;
}

QVariant DbDialog::getValueFrom(DbPluginOption::Type type, QWidget *editor)
{
    QVariant value;
    switch (type)
    {
        case DbPluginOption::STRING:
            value = dynamic_cast<QLineEdit*>(editor)->text();
            break;
        case DbPluginOption::INT:
            value = dynamic_cast<QSpinBox*>(editor)->value();
            break;
        case DbPluginOption::FILE:
            value = dynamic_cast<QLineEdit*>(editor)->text();
            break;
        case DbPluginOption::BOOL:
            value = dynamic_cast<QCheckBox*>(editor)->isChecked();
            break;
        case DbPluginOption::DOUBLE:
            value = dynamic_cast<QDoubleSpinBox*>(editor)->value();
            break;
        default:
            // TODO plugin based handling of custom editors
            break;
    }
    return value;
}

void DbDialog::setValueFor(DbPluginOption::Type type, QWidget *editor, const QVariant &value)
{
    switch (type)
    {
        case DbPluginOption::STRING:
            dynamic_cast<QLineEdit*>(editor)->setText(value.toString());
            break;
        case DbPluginOption::INT:
            dynamic_cast<QSpinBox*>(editor)->setValue(value.toInt());
            break;
        case DbPluginOption::FILE:
            dynamic_cast<QLineEdit*>(editor)->setText(value.toString());
            break;
        case DbPluginOption::BOOL:
            dynamic_cast<QCheckBox*>(editor)->setChecked(value.toBool());
            break;
        case DbPluginOption::DOUBLE:
            dynamic_cast<QDoubleSpinBox*>(editor)->setValue(value.toDouble());
            break;
        default:
            // TODO plugin based handling of custom editors
            break;
    }
}

void DbDialog::updateType()
{
    QFileInfo file(ui->fileEdit->text());
    if (!file.exists() || file.isDir())
    {
        ui->typeCombo->setEnabled(true);
        return;
    }

    DbPlugin* validPlugin = nullptr;
    QHash<QString,QVariant> options;
    QString path = ui->fileEdit->text();
    Db* probeDb = nullptr;
    foreach (DbPlugin* plugin, dbPlugins)
    {
        probeDb = plugin->getInstance("", path, options);
        if (probeDb)
        {
            delete probeDb;
            probeDb = nullptr;

            validPlugin = plugin;
            break;
        }
    }

    if (validPlugin)
        ui->typeCombo->setCurrentText(validPlugin->getLabel());

    ui->typeCombo->setEnabled(!validPlugin);
}

QHash<QString, QVariant> DbDialog::collectOptions()
{
    QHash<QString, QVariant> options;
    foreach (QString key, optionKeyToWidget.keys())
        options[key] = getValueFrom(optionKeyToType[key], optionKeyToWidget[key]);

    DbPlugin* plugin = nullptr;
    if (dbPlugins.count() > 0)
    {
        plugin = dbPlugins[ui->typeCombo->currentIndex()];
        options["plugin"] = plugin->getName();
    }

    return options;
}

bool DbDialog::testDatabase()
{
    QString path = ui->fileEdit->text();
    bool existed = QFile::exists(path);
    bool res = getDb() != nullptr;
    if (!existed)
    {
        QFile file(path);
        file.remove();
    }
    return res;
}

bool DbDialog::validate()
{
    if (ui->fileEdit->text().isEmpty())
        return false;

    if (ui->nameEdit->text().isEmpty())
        return false;

    if (ui->typeCombo->count() == 0)
        return false;

    if (ui->typeCombo->currentIndex() < 0)
        return false;

    return true;
}

void DbDialog::updateState()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validate());
}

void DbDialog::propertyChanged()
{
    ui->testConnIcon->setVisible(false);
}

void DbDialog::typeChanged(int index)
{
    UNUSED(index);
    updateOptions();
    updateState();
}

void DbDialog::valueForNameGenerationChanged()
{
    if (!ui->generateCheckBox->isChecked())
    {
        updateState();
        return;
    }

    DbPlugin* plugin = nullptr;
    if (dbPlugins.count() > 0)
    {
        plugin = dbPlugins[ui->typeCombo->currentIndex()];
        QString generatedName = plugin->generateDbName(ui->fileEdit->text());
        generatedName = generateUniqueName(generatedName, existingDatabaseNames);
        ui->nameEdit->setText(generatedName);
    }
}

void DbDialog::browseForFile()
{
    QString path = QFileDialog::getOpenFileName();
    if (path.isNull())
        return;

    QString key = helperToKey[dynamic_cast<QWidget*>(sender())];
    setValueFor(optionKeyToType[key], optionKeyToWidget[key], path);
}

void DbDialog::generateNameSwitched(bool checked)
{
    if (checked)
    {
        ui->nameEdit->setPlaceholderText(tr("The name will be auto-generated"));
        valueForNameGenerationChanged();
    }
    else
    {
        ui->nameEdit->setPlaceholderText(tr("Type the name"));
    }

    ui->nameEdit->setReadOnly(checked);
}

void DbDialog::fileChanged(const QString &arg1)
{
    UNUSED(arg1);
    valueForNameGenerationChanged();
    updateType();
    propertyChanged();
}

void DbDialog::browseClicked()
{
    QFileInfo fileInfo(ui->fileEdit->text());
    QString dir;
    if (fileInfo.exists() && fileInfo.isFile())
        dir = fileInfo.absolutePath();
    else if (fileInfo.dir().exists())
        dir = fileInfo.dir().absolutePath();

    QString path = getDbPath(dir);
    if (path.isNull())
        return;

    ui->fileEdit->setText(path);
    updateState();
}

void DbDialog::testConnectionClicked()
{
    ui->testConnIcon->setPixmap(testDatabase() ? ICONS.TEST_CONN_OK : ICONS.TEST_CONN_ERROR);
    ui->testConnIcon->setVisible(true);
}

void DbDialog::dbTypeChanged(int index)
{
    typeChanged(index);
    propertyChanged();
}

void DbDialog::nameModified(const QString &arg1)
{
    UNUSED(arg1);
    updateState();
}

void DbDialog::accept()
{
    QString name = getName();
    QString path = getPath();
    QHash<QString, QVariant> options = collectOptions();
    bool perm = isPermanent();
    bool result;
    if (mode == ADD)
        result = DBLIST->addDb(name, path, options, perm);
    else
        result = DBLIST->updateDb(db, name, path, options, perm);

    if (result)
        QDialog::accept();
}
