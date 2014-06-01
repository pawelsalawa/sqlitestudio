#include "populateconfigdialog.h"
#include "ui_populateconfigdialog.h"
#include "plugins/populateplugin.h"
#include "formmanager.h"
#include <QDebug>
#include <QSpinBox>

PopulateConfigDialog::PopulateConfigDialog(PopulateEngine* engine, const QString& column, const QString& pluginName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PopulateConfigDialog),
    engine(engine)
{
    ui->setupUi(this);

    QString headerString = tr("Configuring <b>%1</b> for column <b>%2</b>").arg(pluginName, column);
    ui->headerLabel->setText(headerString );
}

PopulateConfigDialog::~PopulateConfigDialog()
{
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

    ui->contents->layout()->addWidget(w);
    adjustSize();
    return QDialog::exec();
}
