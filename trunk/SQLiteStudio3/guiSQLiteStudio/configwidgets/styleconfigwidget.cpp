#include "styleconfigwidget.h"
#include "uiconfig.h"
#include "common/unused.h"
#include "dialogs/configdialog.h"
#include <QDebug>
#include <QComboBox>

StyleConfigWidget::StyleConfigWidget()
{
}

bool StyleConfigWidget::isConfigForWidget(CfgEntry* key, QWidget* widget)
{
    UNUSED(widget);
    return (key == &CFG_UI.General.Style);
}

void StyleConfigWidget::applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value)
{
    UNUSED(key);

    QComboBox* combo = qobject_cast<QComboBox*>(widget);

    // QComboBox fails to findIndex() in case insensitive manner, so we will do it manually:
    QStringList items;
    for (int i = 0; i < combo->count(); i++)
        items << combo->itemText(i).toLower();

    int idx = items.indexOf(value.toString().toLower());
    combo->setCurrentIndex(idx);
}

QVariant StyleConfigWidget::getWidgetConfigValue(QWidget* widget, bool& ok)
{
    QComboBox* combo = qobject_cast<QComboBox*>(widget);
    if (!combo)
    {
        ok = false;
        return QVariant();
    }

    ok = true;
    return combo->currentText();
}

const char* StyleConfigWidget::getModifiedNotifier() const
{
    return SIGNAL(currentTextChanged(QString));
}

QString StyleConfigWidget::getFilterString(QWidget *widget) const
{
    return ConfigDialog::getFilterString(widget);
}
