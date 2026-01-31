#include "multieditorfk.h"
#include "datagrid/fkcombobox.h"
#include "datagrid/sqlquerymodel.h"
#include "datagrid/sqlqueryview.h"
#include "common/utils_sql.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>

MultiEditorFk::MultiEditorFk(QWidget* parent)
    : MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());

    QWidget* comboContainer = new QWidget();
    QHBoxLayout* comboContainerLayout = new QHBoxLayout();
    comboContainerLayout->setContentsMargins(0, 0, 0, 0);
    comboContainer->setLayout(comboContainerLayout);
    layout()->addWidget(comboContainer);

    comboBox = new FkComboBox();
    comboBox->setEditable(false);
    comboContainerLayout->addWidget(comboBox, 0, Qt::AlignLeft);

    QLabel* label = new QLabel(tr("Selected value preview"));
    layout()->addWidget(label);

    SqlQueryView* preview = new SqlQueryView();
    preview->setReadOnly(true);
    preview->setSimpleBrowserMode(true);
    layout()->addWidget(preview);

    previewModel = new SqlQueryModel(this);
    previewModel->setAsyncMode(false);
    previewModel->setView(preview);
    preview->setModel(previewModel);
    preview->setColumnHidden(0, true); // Hide the first column which is the FK indicator column.);

    connect(comboBox, SIGNAL(valueModified()), this, SIGNAL(valueModified()));
    connect(comboBox, SIGNAL(valueModified()), this, SLOT(updatePreview()));

    setFocusProxy(comboBox);
}

void MultiEditorFk::initFkCombo(Db* db, SqlQueryModelColumn* columnModel)
{
    comboBox->init(db, columnModel);
    previewModel->setDb(db);
    this->db = db;
    this->columnModel = columnModel;
}

void MultiEditorFk::setValue(const QVariant& value)
{
    comboBox->setValue(value);
    updatePreview(value);
}

QVariant MultiEditorFk::getValue()
{
    return comboBox->getValue();
}

void MultiEditorFk::setReadOnly(bool value)
{
    comboBox->setDisabled(value);
}

void MultiEditorFk::focusThisWidget()
{
    comboBox->setFocus();
}

QList<QWidget*> MultiEditorFk::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << comboBox;
    return list;
}

void MultiEditorFk::updatePreview(const QVariant& value)
{
    static_qstring(queryTpl, "SELECT * FROM (%1) WHERE %2 = true");
    QString currColName;
    QString query = FkComboBox::getSqlForFkEditor(db, columnModel, value, &currColName);
    if (query.isNull())
        return;

    QString queryMatching = queryTpl.arg(query, wrapObjIfNeeded(currColName));

    previewModel->setQuery(queryMatching);
    previewModel->executeQuery();
}

void MultiEditorFk::updatePreview()
{
    bool ok;
    QVariant value = comboBox->getValue(nullptr, &ok);
    if (!ok)
        return;

    updatePreview(value);
}
