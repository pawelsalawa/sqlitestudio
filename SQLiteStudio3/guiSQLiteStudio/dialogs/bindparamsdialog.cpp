#include "bindparamsdialog.h"
#include "ui_bindparamsdialog.h"
#include "common/bindparam.h"
#include "multieditor/multieditor.h"
#include "widgetresizer.h"
#include "services/pluginmanager.h"
#include "multieditor/multieditorwidgetplugin.h"

#include <multieditor/multieditorwidget.h>

BindParamsDialog::BindParamsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindParamsDialog)
{
    init();
}

BindParamsDialog::~BindParamsDialog()
{
    delete ui;
}

void BindParamsDialog::setBindParams(const QVector<BindParam*>& params)
{
    bindParams = params;
    initEditors();
}

void BindParamsDialog::init()
{
    ui->setupUi(this);

    contents = new QWidget();
    QVBoxLayout *contentsLayout = new QVBoxLayout();
    contentsLayout->setSpacing(spacing);
    contentsLayout->setMargin(margins);
    contents->setLayout(contentsLayout);

    ui->scrollArea->setWidget(contents);
}

void BindParamsDialog::initEditors()
{
    for (BindParam* param : bindParams)
        initEditor(param);
}

void BindParamsDialog::initEditor(BindParam* param)
{
    // Label
    static_qstring(nameTpl, "[%1] %2");
    QString label = nameTpl.arg(param->position + 1).arg(param->originalName);

    // MultiEditor
    MultiEditor* multiEditor = new MultiEditor(this, MultiEditor::DYNAMIC);
    multiEditor->setReadOnly(false);
    multiEditor->setCornerLabel(label);
    contents->layout()->addWidget(multiEditor);
    editors[param] = multiEditor;

    // MultiEditor editors
    MultiEditorWidgetPlugin* plugin = dynamic_cast<MultiEditorWidgetPlugin*>(PLUGINS->getLoadedPlugin("MultiEditorNumericPlugin"));
    MultiEditorWidget* editor = plugin->getInstance();
    editor->setTabLabel(plugin->getTabLabel());
    multiEditor->addEditor(editor);

    plugin = dynamic_cast<MultiEditorWidgetPlugin*>(PLUGINS->getLoadedPlugin("MultiEditorTextPlugin"));
    editor = plugin->getInstance();
    editor->setTabLabel(plugin->getTabLabel());
    multiEditor->addEditor(editor);

//    multiEditor->setDataType(DataType("NUMERIC"));

    // Resizer
    WidgetResizer* resizer = new WidgetResizer(Qt::Vertical);
    resizer->setWidget(multiEditor);
    resizer->setWidgetMinimumSize(0, minimumFieldHeight);
    contents->layout()->addWidget(resizer);
}

void BindParamsDialog::accept()
{
    for (BindParam* param : bindParams)
        param->value = editors[param]->getValue();

    QDialog::accept();
}
