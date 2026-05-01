#include "bindparamsdialog.h"
#include "ui_bindparamsdialog.h"
#include "common/bindparam.h"
#include "multieditor/multieditor.h"
#include "widgetresizer.h"
#include "services/pluginmanager.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "multieditor/multieditorwidget.h"
#include "services/config.h"
#include <QDebug>
#include <QLineEdit>

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

    QVector<QPair<QString, QVariant>> savedParams = getSavedParams();
    bool multi = needsMultiEditor(savedParams);
    if (multi)
        initMultiEditors(savedParams);
    else
        initSimpleEditors(savedParams);

    ui->advancedModeCheck->setChecked(multi);
    connect(ui->advancedModeCheck, &QCheckBox::toggled, this, &BindParamsDialog::toggleMode);
}

void BindParamsDialog::init()
{
    ui->setupUi(this);

    contents = new QWidget();
    ui->scrollArea->setWidget(contents);
    ui->scrollArea->setAlignment(Qt::AlignTop);
}

void BindParamsDialog::initSimpleEditors(const QVector<QPair<QString, QVariant>>& savedParams)
{
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(simpleSpacing);
    grid->setContentsMargins(simpleMargins, simpleMargins, simpleMargins, simpleMargins);
    grid->setAlignment(Qt::AlignTop);
    contents->setLayout(grid);

    QFont font = QLineEdit().font();
    font.setBold(true);

    static_qstring(nameTpl, "[%1]");
    QLineEdit* firstEditor = nullptr;
    int rowIdx = 0;
    for (BindParam* param : bindParams)
    {
        QString savedValue = savedParams.size() > param->position ? savedParams[param->position].second.toString() : "";

        QLabel* label = new QLabel(this);
        label->setFont(font);
        label->setText(param->originalName == "?" ? nameTpl.arg(param->position + 1) : param->originalName);
        grid->addWidget(label, rowIdx, 0);

        QLineEdit* edit = new QLineEdit(this);
        edit->setText(savedValue);
        grid->addWidget(edit, rowIdx, 1);
        simpleEditors[param] = edit;

        if (firstEditor == nullptr)
            firstEditor = edit;

        rowIdx++;
    }

    if (firstEditor)
        firstEditor->setFocus();
}

void BindParamsDialog::initMultiEditors(const QVector<QPair<QString, QVariant>>& savedParams)
{
    QVBoxLayout *contentsLayout = new QVBoxLayout();
    contentsLayout->setSpacing(multiSpacing);
    contentsLayout->setContentsMargins(multiMargins, multiMargins, multiMargins, multiMargins);
    contentsLayout->setAlignment(Qt::AlignTop);
    contents->setLayout(contentsLayout);

    MultiEditor* firstEditor = nullptr;
    MultiEditor* multiEditor = nullptr;
    for (BindParam*& param : bindParams)
    {
        QVariant savedValue = savedParams.size() > param->position ? savedParams[param->position].second : QVariant();
        multiEditor = initMultiEditor(param, savedValue);
        if (firstEditor == nullptr)
            firstEditor = multiEditor;
    }

    if (firstEditor)
        firstEditor->focusThisEditor();

    int minWidth = 0;
    for (auto&& editor : multiEditors.values())
        minWidth = qMax(minWidth, editor->getCornerLabelWidth());

    for (auto&& editor : multiEditors.values())
        editor->adjustCornerLabelMinWidth(minWidth);
}

MultiEditor* BindParamsDialog::initMultiEditor(BindParam* param, const QVariant& cachedValue)
{
    // Label
    static_qstring(nameTpl, "[%1]");
    QString label = param->originalName == "?" ? nameTpl.arg(param->position + 1) : param->originalName;

    // MultiEditor
    MultiEditor* multiEditor = new MultiEditor(this, MultiEditor::DYNAMIC);
    multiEditor->setReadOnly(false);
    multiEditor->setSaveButtonVisible(false);
    multiEditor->setCornerLabel(label);
    contents->layout()->addWidget(multiEditor);
    contents->layout()->setAlignment(multiEditor, Qt::AlignTop);
    multiEditors[param] = multiEditor;

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
        switch (cachedValue.userType())
        {
            case QMetaType::LongLong:
            case QMetaType::ULongLong:
            case QMetaType::Int:
            case QMetaType::UInt:
            case QMetaType::Double:
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

QVector<QPair<QString, QVariant>> BindParamsDialog::collectCurrentValues() const
{
    QVector<QPair<QString, QVariant>> values;
    values.reserve(bindParams.size());
    if (!multiEditors.isEmpty())
    {
        for (BindParam* param : bindParams)
            values << QPair<QString, QVariant>(param->originalName, multiEditors[param]->getValue());
    }
    else if (!simpleEditors.isEmpty())
    {
        for (BindParam* param : bindParams)
            values << QPair<QString, QVariant>(param->originalName, simpleEditors[param]->text());
    }
    return values;
}

void BindParamsDialog::clearCurrentEditors()
{
    QLayout* layout = contents->layout();
    if (layout)
    {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr)
        {
            if (QWidget* widget = item->widget())
                delete widget;

            delete item;
        }
        delete layout;
    }
    multiEditors.clear();
    simpleEditors.clear();
}

void BindParamsDialog::toggleMode(bool advanced)
{
    QVector<QPair<QString, QVariant>> values = collectCurrentValues();
    clearCurrentEditors();
    if (advanced)
        initMultiEditors(values);
    else
        initSimpleEditors(values);
}

bool BindParamsDialog::needsMultiEditor(const QVector<QPair<QString, QVariant>>& savedParams) const
{
    for (BindParam* param : bindParams)
    {
        if (param->position >= savedParams.size())
            continue;

        QVariant value = savedParams[param->position].second;
        if (!value.isValid())
            continue;

        if (value.userType() != QMetaType::QString)
            return true;

        QString str = value.toString();
        if (str.length() > 200 || str.contains('\n') || str.contains(QChar::ParagraphSeparator) || str.contains(QChar::LineSeparator))
            return true;
    }
    return false;
}

QVector<QPair<QString, QVariant> > BindParamsDialog::getSavedParams() const
{
    QStringList paramNames = bindParams | MAP(param, {return param->originalName;});
    return CFG->getBindParamHistory(paramNames);
}

void BindParamsDialog::accept()
{
    QVector<QPair<QString, QVariant>> paramHistory;
    paramHistory.reserve(bindParams.size());
    bool rememberValue = false;
    QVariant emptyValue;
    for (BindParam* param : bindParams)
    {
        param->value = ui->advancedModeCheck->isChecked() ? multiEditors[param]->getValue() : simpleEditors[param]->text();
        rememberValue = (param->value.userType() != QMetaType::QByteArray ||
                         param->value.toByteArray().size() <= 102400);
        paramHistory << QPair<QString, QVariant>(param->originalName, rememberValue ? param->value : emptyValue);
    }

    CFG->addBindParamHistory(paramHistory);

    QDialog::accept();
}
