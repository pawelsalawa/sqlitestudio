#ifndef MULTIEDITORFK_H
#define MULTIEDITORFK_H

#include "guiSQLiteStudio_global.h"
#include "multieditorwidget.h"

class SqlQueryModel;
class Db;
class SqlQueryModelColumn;
class FkComboBox;

class GUI_API_EXPORT MultiEditorFk : public MultiEditorWidget
{
    Q_OBJECT

    public:
        explicit MultiEditorFk(QWidget *parent = nullptr);

        void initFkCombo(Db* db, SqlQueryModelColumn* columnModel);
        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool value);
        void focusThisWidget();

        QList<QWidget*> getNoScrollWidgets();

    private:
        void updatePreview(const QVariant& value);

        FkComboBox* comboBox = nullptr;
        Db* db = nullptr;
        SqlQueryModelColumn* columnModel = nullptr;
        SqlQueryModel* previewModel = nullptr;

    private slots:
        void updatePreview();
};

#endif // MULTIEDITORFK_H
