#ifndef MULTIEDITORBOOL_H
#define MULTIEDITORBOOL_H

#include "multieditorwidget.h"
#include "multieditorwidgetplugin.h"
#include "plugins/genericplugin.h"
#include <QStringList>

class QCheckBox;

class MultiEditorBool : public MultiEditorWidget
{
        Q_OBJECT

    public:
        explicit MultiEditorBool(QWidget* parent = 0);

        static void staticInit();

        void setValue(const QVariant& boolValue);
        QVariant getValue();
        void setReadOnly(bool boolValue);
        QList<QWidget*> getNoScrollWidgets();
        QString getTabLabel();

    private:
        enum Format
        {
            TRUE_FALSE,
            ON_OFF,
            YES_NO,
            ONE_ZERO,
            BOOL
        };

        bool valueFromString(const QString& strValue);
        void updateLabel();

        static QStringList validValues;

        QCheckBox* checkBox;
        Format valueFormat = ONE_ZERO;
        bool upperCaseValue = false;
        bool readOnly = false;
        bool boolValue = false;

    private slots:
        void stateChanged(int state);
};

class MultiEditorBoolPlugin : public GenericPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")
    SQLITESTUDIO_PLUGIN_DESC("Boolean data editor.")
    SQLITESTUDIO_PLUGIN_TITLE("Boolean")
    SQLITESTUDIO_PLUGIN_VERSION(10000)

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const DataType& dataType);
        int getPriority(const DataType& dataType);
};

#endif // MULTIEDITORBOOL_H
