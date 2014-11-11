#include "uiconfig.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QStyle>
#include <QStandardItem>
#include <QDir>

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
#ifdef Q_OS_WIN32
        font.setPointSize(font.pointSize() - 1);
#else
        font.setPointSize(font.pointSize() - 2);
#endif
        return font;
    }

}

CFG_DEFINE(Ui)

void setFileDialogInitPathByFile(const QString& filePath)
{
    if (filePath.isNull())
        return;

    QDir newDir(filePath);
    newDir.cdUp();
    setFileDialogInitPath(newDir.absolutePath());
}

void setFileDialogInitPath(const QString& path)
{
    CFG_UI.General.FileDialogLastPath.set(path);
}

QString getFileDialogInitPath()
{
    return CFG_UI.General.FileDialogLastPath.get();
}
