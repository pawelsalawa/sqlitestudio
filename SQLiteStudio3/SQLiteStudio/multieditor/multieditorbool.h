#ifndef MULTIEDITORBOOL_H
#define MULTIEDITORBOOL_H

#include "multieditorwidget.h"
#include <QStringList>

class QCheckBox;

class MultiEditorBool : public MultiEditorWidget
{
    public:
        explicit MultiEditorBool(QWidget* parent = 0);

        static void staticInit();

        void setValue(const QVariant& boolValue);
        QVariant getValue();
        void setReadOnly(bool boolValue);
        QList<QWidget*> getNoScrollWidgets();

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

#endif // MULTIEDITORBOOL_H
