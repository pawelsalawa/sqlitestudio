#ifndef MULTIEDITORDIALOG_H
#define MULTIEDITORDIALOG_H

#include "datagrid/sqlquerymodelcolumn.h"
#include "guiSQLiteStudio_global.h"
#include <QDialog>

class MultiEditor;
class QDialogButtonBox;

class GUI_API_EXPORT MultiEditorDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit MultiEditorDialog(QWidget *parent = 0);
        ~MultiEditorDialog();

        void setValue(const QVariant& value, const DataType& dataType);
        void setValue(const QVariant& value);
        QVariant getValue() const;
        void setNewRow(bool value);
        bool isNewRow() const;
        void setUntouched(bool value);
        bool isUntouched() const;

        void setDataType(const DataType& dataType);
        void setReadOnly(bool readOnly);
        void enableFk(Db* db, SqlQueryModelColumn* column);
        void configureResetButton(SqlQueryModelColumn* column);

    private:
        MultiEditor* multiEditor = nullptr;
        QDialogButtonBox* buttonBox = nullptr;
};

#endif // MULTIEDITORDIALOG_H
