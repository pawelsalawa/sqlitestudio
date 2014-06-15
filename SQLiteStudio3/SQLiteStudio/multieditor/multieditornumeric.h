#ifndef MULTIEDITORNUMERIC_H
#define MULTIEDITORNUMERIC_H

#include "multieditorwidget.h"
#include "multieditorwidgetplugin.h"
#include "plugins/genericplugin.h"

class NumericSpinBox;

class MultiEditorNumeric : public MultiEditorWidget
{
    public:
        explicit MultiEditorNumeric(QWidget *parent = 0);

        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool value);
        QString getTabLabel();

        QList<QWidget*> getNoScrollWidgets();

    private:
        NumericSpinBox* spinBox;
};

class MultiEditorNumericPlugin : public GenericPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")
    SQLITESTUDIO_PLUGIN_DESC("Numeric data editor.")
    SQLITESTUDIO_PLUGIN_TITLE("Numeric types")
    SQLITESTUDIO_PLUGIN_VERSION(10000)

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const DataType& dataType);
        int getPriority(const DataType& dataType);
};

#endif // MULTIEDITORNUMERIC_H
