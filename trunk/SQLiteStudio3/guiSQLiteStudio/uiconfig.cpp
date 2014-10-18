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
#ifdef Q_OS_MACX
        font.setFamily("Courier New");
#elif defined(Q_OS_WIN32)
        font.setFamily("Consolas");
#else
        font.setFamily("DejaVu Sans Mono");
#endif
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
