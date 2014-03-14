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
    ui->setupUi(this);
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
        Q_ASSERT(idx > -1); // if we have db of certain type, it must be driven by existing db plugin
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
    ui->browseLocalButton->setIcon(ICON("database_file"));
    ui->browseRemoteButton->setIcon(ICON("database_network"));
    dbPlugins = PLUGINS->getLoadedPlugins<DbPlugin>();
    int remotes = 0;
    int locals = 0;
    foreach (DbPlugin* dbPlugin, dbPlugins)
    {
        ui->typeCombo->addItem(dbPlugin->getLabel());
        if (dbPlugin->isRemote())
            remotes++;
        else
            locals++;
    }

    ui->browseRemoteButton->setVisible(remotes > 0);
    ui->browseLocalButton->setVisible(locals > 0);
    ui->testIcon->setVisible(false);
    on_generateCheckBox_toggled(true);
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
        plugin = dbPlugins[ui->typeCombo->currentIndex()];
        QList<DbPluginOption> optList = plugin->getOptionsList();
        if (optList.size() > 0)
        {
            // Add new options
            int row = 3;
            foreach (DbPluginOption opt, optList)
                addOption(opt, row++);
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
    return getDb() != nullptr;
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
    ui->testIcon->setVisible(false);
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

void DbDialog::nameChanged()
{

}

void DbDialog::browseForFile()
{
    QString path = QFileDialog::getOpenFileName();
    if (path.isNull())
        return;

    QString key = helperToKey[dynamic_cast<QWidget*>(sender())];
    setValueFor(optionKeyToType[key], optionKeyToWidget[key], path);
}

void DbDialog::on_generateCheckBox_toggled(bool checked)
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

void DbDialog::on_fileEdit_textChanged(const QString &arg1)
{
    UNUSED(arg1);
    valueForNameGenerationChanged();
    updateType();
    propertyChanged();
}

void DbDialog::on_browseLocalButton_clicked()
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
}

void DbDialog::on_browseRemoteButton_clicked()
{
    // TODO
}

void DbDialog::on_testConnButton_clicked()
{
    QIcon* icon = testDatabase() ? ICON_PTR("test_conn_ok") : ICON_PTR("test_conn_error");
    ui->testIcon->setPixmap(icon->pixmap(icon->availableSizes()[0]));
    ui->testIcon->setVisible(true);
}

void DbDialog::on_typeCombo_activated(int index)
{
    typeChanged(index);
    propertyChanged();
}

void DbDialog::on_nameEdit_textChanged(const QString &arg1)
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
