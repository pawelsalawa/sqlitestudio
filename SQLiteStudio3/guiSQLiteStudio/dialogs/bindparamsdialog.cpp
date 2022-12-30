#include "bindparamsdialog.h"
#include "ui_bindparamsdialog.h"
#include "common/bindparam.h"
#include "multieditor/multieditor.h"
#include "widgetresizer.h"
#include "services/pluginmanager.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "multieditor/multieditorwidget.h"
#include <QDebug>

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
    contentsLayout->setAlignment(Qt::AlignTop);
    contents->setLayout(contentsLayout);

    ui->scrollArea->setWidget(contents);
    ui->scrollArea->setAlignment(Qt::AlignTop);
}

void BindParamsDialog::initEditors()
{
    QStringList paramNames;
    for (BindParam*& param : bindParams)
        paramNames << param->originalName;

    MultiEditor* firstEditor = nullptr;
    MultiEditor* multiEditor = nullptr;
    QVector<QPair<QString, QVariant>> paramHistory = CFG->getBindParamHistory(paramNames);
    for (BindParam*& param : bindParams)
    {
        multiEditor = initEditor(param, paramHistory.size() > param->position ? paramHistory[param->position].second : QVariant());
        if (firstEditor == nullptr)
            firstEditor = multiEditor;
    }

    if (firstEditor)
        firstEditor->focusThisEditor();
}

MultiEditor* BindParamsDialog::initEditor(BindParam* param, const QVariant& cachedValue)
{
    // Label
    static_qstring(nameTpl, "[%1] %2");
    QString label = nameTpl.arg(param->position + 1).arg(param->originalName);

    // MultiEditor
    MultiEditor* multiEditor = new MultiEditor(this, MultiEditor::DYNAMIC);
    multiEditor->setReadOnly(false);
    multiEditor->setCornerLabel(label);
    contents->layout()->addWidget(multiEditor);
    contents->layout()->setAlignment(multiEditor, Qt::AlignTop);
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

    // Resizer
    WidgetResizer* resizer = new WidgetResizer(Qt::Vertical);
    resizer->setWidget(multiEditor);
    resizer->setWidgetMinimumSize(0, minimumFieldHeight);
    contents->layout()->addWidget(resizer);
    resizer->minimizeHeight();

    if (cachedValue.isValid())
    {
        switch (cachedValue.type())
        {
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Double:
                multiEditor->showTab(0);
                break;
            default:
                multiEditor->showTab(1);
                break;
        }

        multiEditor->setValue(cachedValue);
    }

    return multiEditor;
}

void BindParamsDialog::accept()
{
    QVector<QPair<QString, QVariant>> paramHistory;
    paramHistory.reserve(bindParams.size());
    bool rememberValue = false;
    QVariant emptyValue;
    for (BindParam* param : bindParams)
    {
        param->value = editors[param]->getValue();
        rememberValue = (param->value.type() != QVariant::ByteArray || param->value.toByteArray().size() <= 102400);
        paramHistory << QPair<QString, QVariant>(param->originalName, rememberValue ? param->value : emptyValue);
    }

    CFG->addBindParamHistory(paramHistory);

    QDialog::accept();
}
