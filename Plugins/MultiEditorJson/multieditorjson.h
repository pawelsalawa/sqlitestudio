#ifndef MULTIEDITORJSON_H
#define MULTIEDITORJSON_H

#include "multieditor/multieditorwidget.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "multieditorjson_global.h"
#include "datatype.h"
#include "plugins/genericplugin.h"
#include "jsonhighlighter.h"
#include <QVariant>
#include <QPlainTextEdit>

class QToolBar;
class QAction;
class QLabel;

class MultiEditorJson : public MultiEditorWidget
{
    Q_OBJECT

public:
    MultiEditorJson();

    void setValue(const QVariant& value) override;
    QVariant getValue() override;
    void setReadOnly(bool value) override;
    QList<QWidget*> getNoScrollWidgets() override;
    void focusThisWidget() override;
    QString getPreferredFileFilter();

    static bool isValidJson(const QString& json, QString* errorMsg = nullptr);

private:
    QString prettifyJson(const QString& json);
    QString condenseJson(const QString& json);
    void updateStatus();

    QPlainTextEdit* textEdit = nullptr;
    JsonHighlighter* highlighter = nullptr;
    QToolBar* toolbar = nullptr;
    QAction* prettifyAction = nullptr;
    QAction* condenseAction = nullptr;
    QAction* validateAction = nullptr;
    QLabel* statusLabel = nullptr;
    bool readOnly = false;

private slots:
    void onPrettify();
    void onCondense();
    void onValidate();
    void onTextChanged();
};

class MULTIEDITORJSON_EXPORT MultiEditorJsonPlugin : public GenericPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN("multieditorjson.json")

public:
    MultiEditorWidget* getInstance() override;
    bool validFor(const DataType& dataType) override;
    int getPriority(const QVariant& value, const DataType& dataType) override;
    QString getTabLabel() override;
    bool init() override;
    void deinit() override;

private:
    QList<MultiEditorJson*> instances;
};

#endif // MULTIEDITORJSON_H
