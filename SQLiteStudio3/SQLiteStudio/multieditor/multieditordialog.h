#ifndef MULTIEDITORDIALOG_H
#define MULTIEDITORDIALOG_H

#include "datagrid/sqlquerymodelcolumn.h"
#include <QDialog>

class MultiEditor;
class QDialogButtonBox;

class MultiEditorDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit MultiEditorDialog(QWidget *parent = 0);
        ~MultiEditorDialog();

        void setValue(const QVariant& value);
        QVariant getValue();

        void setDataType(const SqlQueryModelColumn::DataType& dataType);
        void setReadOnly(bool readOnly);

    private:
        MultiEditor* multiEditor;
        QDialogButtonBox* buttonBox;
};

#endif // MULTIEDITORDIALOG_H
