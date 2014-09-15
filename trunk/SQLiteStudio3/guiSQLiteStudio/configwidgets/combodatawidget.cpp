#include "combodatawidget.h"
#include "common/unused.h"
#include "config_builder.h"
#include "dialogs/configdialog.h"
#include <QComboBox>
#include <QDebug>

ComboDataWidget::ComboDataWidget(CfgEntry* key) :
    assignedKey(key)
{
}

bool ComboDataWidget::isConfigForWidget(CfgEntry* key, QWidget* widget)
{
    UNUSED(widget);
    return (assignedKey == key);
}

void ComboDataWidget::applyConfigToWidget(CfgEntry* key, QWidget* widget, const QVariant& value)
{
    QComboBox* cb = dynamic_cast<QComboBox*>(widget);
    if (!cb)
    {
        qWarning() << "ComboDataWidget assigned to widget which is not combobox, but:" << widget->metaObject()->className()
                      << ", config key:" << key->getFullKey();
        return;
    }

    QVariant data;
    for (int i = 0; i < cb->count(); i++)
    {
        data = cb->itemData(i);
        if (data == value)
        {
            cb->setCurrentIndex(i);
            break;
        }
    }
}

QVariant ComboDataWidget::getWidgetConfigValue(QWidget* widget, bool& ok)
{
    QComboBox* cb = dynamic_cast<QComboBox*>(widget);
    if (!cb)
    {
        ok = false;
        qWarning() << "ComboDataWidget assigned to widget which is not combobox, but:" << widget->metaObject()->className();
        return QVariant();
    }

    ok = true;
    return cb->itemData(cb->currentIndex());
}

const char* ComboDataWidget::getModifiedNotifier() const
{
    return SIGNAL(currentTextChanged(QString));
}

QString ComboDataWidget::getFilterString(QWidget *widget) const
{
    return ConfigDialog::getFilterString(widget);
}
