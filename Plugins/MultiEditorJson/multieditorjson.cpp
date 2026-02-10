#include "multieditorjson.h"
#include "iconmanager.h"
#include "services/notifymanager.h"
#include "uiconfig.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFont>
#include <multieditor/multieditor.h>

MultiEditorJson::MultiEditorJson()
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    // Create toolbar
    toolbar = new QToolBar();
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setProperty(MainWindow::CONSTANT_ICON_SIZE, true);

    prettifyAction = toolbar->addAction(ICONS.FORMAT_SQL, tr("Prettify"), this, SLOT(onPrettify()));
    prettifyAction->setToolTip(tr("Format JSON with indentation"));
    
    condenseAction = toolbar->addAction(ICONS.ERASE, tr("Condense"), this, SLOT(onCondense()));
    condenseAction->setToolTip(tr("Remove whitespace and format on single line"));
    
    toolbar->addSeparator();
    
    validateAction = toolbar->addAction(ICONS.CONSTRAINT_CHECK, tr("Validate"), this, SLOT(onValidate()));
    validateAction->setToolTip(tr("Validate JSON syntax"));
    
    toolbar->addSeparator();
    
    // Status label
    statusLabel = new QLabel();
    statusLabel->setStyleSheet("QLabel { padding: 0 5px; }");
    toolbar->addWidget(statusLabel);
    
    mainLayout->addWidget(toolbar);

    // Create text editor
    textEdit = new QPlainTextEdit();
    
    // Set monospace font
    textEdit->setFont(CFG_UI.Fonts.SqlEditor.get());
    
    // Enable line wrapping for better readability
    textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    
    mainLayout->addWidget(textEdit);

    // Create syntax highlighter
    highlighter = new JsonHighlighter(textEdit->document());

    // Connect signals
    connect(textEdit, &QPlainTextEdit::textChanged, this, &MultiEditorJson::onTextChanged);
    connect(textEdit, &QPlainTextEdit::textChanged, this, &MultiEditorWidget::valueModified);
}

void MultiEditorJson::setValue(const QVariant& value)
{
    QString text = value.toString();
    
    // Disconnect to avoid triggering valueModified
    disconnect(textEdit, &QPlainTextEdit::textChanged, this, &MultiEditorWidget::valueModified);
    
    textEdit->setPlainText(text);
    updateStatus();
    
    // Reconnect
    connect(textEdit, &QPlainTextEdit::textChanged, this, &MultiEditorWidget::valueModified);
}

QVariant MultiEditorJson::getValue()
{
    QString newStr = textEdit->document()->toRawText();
    newStr.replace(QChar::ParagraphSeparator, '\n');
    newStr.replace(QChar::LineSeparator, '\n');
    return newStr;
}

void MultiEditorJson::setReadOnly(bool value)
{
    readOnly = value;
    textEdit->setReadOnly(value);
    prettifyAction->setEnabled(!value);
    condenseAction->setEnabled(!value);
}

QList<QWidget*> MultiEditorJson::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << textEdit;
    return list;
}

void MultiEditorJson::focusThisWidget()
{
    textEdit->setFocus();
}

QString MultiEditorJson::getPreferredFileFilter()
{
    return tr("JSON files (*.json, *.txt)");
}

bool MultiEditorJson::isValidJson(const QString& json, QString* errorMsg)
{
    if (json.trimmed().isEmpty())
        return true; // Empty is considered valid
    
    QJsonParseError error;
    QJsonDocument::fromJson(json.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError)
    {
        if (errorMsg)
            *errorMsg = error.errorString();
        return false;
    }
    
    return true;
}

QString MultiEditorJson::prettifyJson(const QString& json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError)
        return json; // Return original if invalid
    
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

QString MultiEditorJson::condenseJson(const QString& json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError)
        return json; // Return original if invalid
    
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

void MultiEditorJson::updateStatus()
{
    QString errorMsg;
    if (isValidJson(textEdit->toPlainText(), &errorMsg))
    {
        statusLabel->setText(tr("✓ Valid JSON"));
        statusLabel->setStyleSheet("QLabel { color: green; padding: 0 5px; }");
    }
    else
    {
        statusLabel->setText(tr("✗ Invalid: %1").arg(errorMsg));
        statusLabel->setStyleSheet("QLabel { color: red; padding: 0 5px; }");
    }
}

void MultiEditorJson::onPrettify()
{
    if (readOnly)
        return;
    
    QString original = textEdit->toPlainText();
    QString prettified = prettifyJson(original);
    
    if (prettified != original)
    {
        textEdit->setPlainText(prettified);
        updateStatus();
    }
}

void MultiEditorJson::onCondense()
{
    if (readOnly)
        return;
    
    QString original = textEdit->toPlainText();
    QString condensed = condenseJson(original);
    
    if (condensed != original)
    {
        textEdit->setPlainText(condensed);
        updateStatus();
    }
}

void MultiEditorJson::onValidate()
{
    QString errorMsg;
    if (isValidJson(textEdit->toPlainText(), &errorMsg))
    {
        notifyInfo(tr("JSON is valid!"));
    }
    else
    {
        notifyWarn(tr("JSON validation failed: %1").arg(errorMsg));
    }
}

void MultiEditorJson::onTextChanged()
{
    updateStatus();
}

// Plugin implementation
MultiEditorWidget* MultiEditorJsonPlugin::getInstance()
{
    MultiEditorJson* instance = new MultiEditorJson();
    instances << instance;
    connect(instance, &QObject::destroyed, [this, instance]()
    {
        instances.removeOne(instance);
    });
    return instance;
}

bool MultiEditorJsonPlugin::validFor(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::TEXT:
        case DataType::STRING:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::BLOB:
        case DataType::ANY:
        case DataType::NONE:
        case DataType::unknown:
            return true;
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            break;
    }
    return false;
}

int MultiEditorJsonPlugin::getPriority(const QVariant& value, const DataType& dataType)
{
    if (value.userType() == QMetaType::QString)
    {
        QString str = value.toString();
        // First quick test for [ or { to avoid unnecessary JSON parsing
        if ((str.startsWith("[") || str.startsWith("{")) && MultiEditorJson::isValidJson(str))
            return 1; // Highest priority for valid JSON strings
    }

    switch (dataType.getType())
    {
        case DataType::TEXT:
        case DataType::STRING:
            return 20; // Higher priority for text fields
        case DataType::BLOB:
            return 30; // Lower priority for blobs
        case DataType::CHAR:
        case DataType::VARCHAR:
            return 25;
        case DataType::NONE:
        case DataType::ANY:
        case DataType::unknown:
            return 50; // Lowest priority for unknown types
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            break;
    }
    return 100;
}

QString MultiEditorJsonPlugin::getTabLabel()
{
    return tr("JSON");
}

bool MultiEditorJsonPlugin::init()
{
    SQLS_INIT_RESOURCE(multieditorjson);
    return GenericPlugin::init();
}

void MultiEditorJsonPlugin::deinit()
{
    for (MultiEditorJson*& editor : instances)
    {
        editor->notifyAboutUnload();
        delete editor;
    }
    SQLS_CLEANUP_RESOURCE(multieditorjson);
    GenericPlugin::deinit();
}
