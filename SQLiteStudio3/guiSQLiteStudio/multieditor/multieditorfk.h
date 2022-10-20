#ifndef MULTIEDITORFK_H
#define MULTIEDITORFK_H

#include "multieditorwidget.h"

class Db;
class SqlQueryModelColumn;
class FkComboBox;

class MultiEditorFk : public MultiEditorWidget
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
        FkComboBox* comboBox = nullptr;
};

#endif // MULTIEDITORFK_H
