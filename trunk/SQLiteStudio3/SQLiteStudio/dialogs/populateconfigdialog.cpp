#include "populateconfigdialog.h"
#include "ui_populateconfigdialog.h"
#include "plugins/populateplugin.h"
#include "services/populatemanager.h"
#include "sqlitestudio.h"
#include "formmanager.h"
#include "configmapper.h"
#include "uiutils.h"
#include <QPushButton>
#include <QDebug>
#include <QSpinBox>

PopulateConfigDialog::PopulateConfigDialog(PopulateEngine* engine, const QString& column, const QString& pluginName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PopulateConfigDialog),
    engine(engine),
    column(column),
    pluginName(pluginName)
{
    init();
}

PopulateConfigDialog::~PopulateConfigDialog()
{
    safe_delete(configMapper);
    delete ui;
}

int PopulateConfigDialog::exec()
{
    QString formName = engine->getPopulateConfigFormName();
    if (formName.isNull())
    {
        qCritical() << "Null form name from populating engine.";
        return QDialog::Rejected;
    }

    QWidget* w = FORMS->createWidget(formName);
    if (!w)
        return QDialog::Rejected;

    configMapper->bindToConfig(w);
    ui->contents->layout()->addWidget(w);
    adjustSize();
    validateEngine();
    return QDialog::exec();
}

void PopulateConfigDialog::init()
{
    ui->setupUi(this);

    QString headerString = tr("Configuring <b>%1</b> for column <b>%2</b>").arg(pluginName, column);
    ui->headerLabel->setText(headerString );

    configMapper = new ConfigMapper(engine->getConfig());
    connect(configMapper, SIGNAL(modified()), this, SLOT(validateEngine()));

    connect(POPULATE_MANAGER, SIGNAL(validationResultFromPlugin(bool,CfgEntry*,QString)), this, SLOT(validationResultFromPlugin(bool,CfgEntry*,QString)));
    connect(POPULATE_MANAGER, SIGNAL(stateUpdateRequestFromPlugin(CfgEntry*,bool,bool)), this, SLOT(stateUpdateRequestFromPlugin(CfgEntry*,bool,bool)));
}

void PopulateConfigDialog::validateEngine()
{
    engine->validateOptions();
}

void PopulateConfigDialog::validationResultFromPlugin(bool valid, CfgEntry* key, const QString& msg)
{
    QWidget* w = configMapper->getBindWidgetForConfig(key);
    if (w)
        setValidState(w, valid, msg);

    if (valid == pluginConfigOk.contains(key)) // if state changed
    {
        if (!valid)
            pluginConfigOk[key] = false;
        else
            pluginConfigOk.remove(key);
    }
    updateState();
}

void PopulateConfigDialog::stateUpdateRequestFromPlugin(CfgEntry* key, bool visible, bool enabled)
{
    QWidget* w = configMapper->getBindWidgetForConfig(key);
    if (!w)
        return;

    w->setVisible(visible);
    w->setEnabled(enabled);
}

void PopulateConfigDialog::updateState()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(pluginConfigOk.size() == 0);
}
