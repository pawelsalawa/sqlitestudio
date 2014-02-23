#ifndef MULTIEDITORNUMERIC_H
#define MULTIEDITORNUMERIC_H

#include "multieditorwidget.h"

class NumericSpinBox;

class MultiEditorNumeric : public MultiEditorWidget
{
    public:
        explicit MultiEditorNumeric(QWidget *parent = 0);

        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool value);

        QList<QWidget*> getNoScrollWidgets();

    private:
        NumericSpinBox* spinBox;

    private slots:
        void modified();
};

#endif // MULTIEDITORNUMERIC_H
