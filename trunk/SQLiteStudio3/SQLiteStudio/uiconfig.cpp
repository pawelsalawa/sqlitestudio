#include "uiconfig.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QStyle>
#include <QStandardItem>

namespace Cfg
{
    QVariant getStyleDefaultValue()
    {
        return QApplication::style()->objectName();
    }

    QVariant getDefaultTextEditorFont()
    {
        QPlainTextEdit monoEdit;
        QFont font = monoEdit.document()->defaultFont();
        font.setFamily("DejaVu Sans Mono");
        return QVariant::fromValue<QFont>(font);
    }

    QVariant getDefaultItemViewFont()
    {
        QStandardItem it;
        return it.font();
    }

    QVariant getDefaultDbTreeLabelFont()
    {
        QFont font = getDefaultItemViewFont().value<QFont>();
        font.setPointSize(font.pointSize() - 2);
        return font;
    }

}

CFG_DEFINE(Ui)
