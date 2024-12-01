#ifndef MULTIEDITORBOOL_H
#define MULTIEDITORBOOL_H

#include "guiSQLiteStudio_global.h"
#include "multieditorwidget.h"
#include "multieditorwidgetplugin.h"
#include "plugins/builtinplugin.h"
#include <QStringList>

class QCheckBox;

class GUI_API_EXPORT MultiEditorBool : public MultiEditorWidget
{
        Q_OBJECT

    public:
        explicit MultiEditorBool(QWidget* parent = 0);

        static void staticInit();

        void setValue(const QVariant& boolValue);
        QVariant getValue();
        void setReadOnly(bool boolValue);
        QList<QWidget*> getNoScrollWidgets();
        void focusThisWidget();

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

        QCheckBox* checkBox = nullptr;
        Format valueFormat = ONE_ZERO;
        bool upperCaseValue = false;
        bool readOnly = false;
        bool boolValue = false;

    private slots:
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
        void stateChanged(int state);
#else
        void stateChanged(Qt::CheckState state);
#endif
};

class GUI_API_EXPORT MultiEditorBoolPlugin : public BuiltInPlugin, public MultiEditorWidgetPlugin
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
        QString getTabLabel();
};

#endif // MULTIEDITORBOOL_H
