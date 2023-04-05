#include "uiconfig.h"
#include "style.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QStyle>
#include <QStandardItem>
#include <QDir>
#include <QDebug>

#define DEFINE_COLOR_HELPER_FN(COLOR_NAME) \
    QColor get##COLOR_NAME() \
    { \
        return CFG_UI.Colors.COLOR_NAME##Custom.get() ? \
            CFG_UI.Colors.COLOR_NAME.get() : \
            getDefault##COLOR_NAME().value<QColor>(); \
    }

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

    QVariant getDefaultSyntaxParenthesisBg()
    {
        return STYLE->standardPalette().windowText();
    }

    QVariant getDefaultSyntaxParenthesisFg()
    {
        return STYLE->standardPalette().window();
    }

    QVariant getDefaultSyntaxCurrentLineBg()
    {
        return STYLE->extendedPalette().editorLineBase();
    }

    QVariant getDefaultSyntaxCurrentQueryBg()
    {
        return STYLE->extendedPalette().editorCurrentQueryBase();
    }

    QVariant getDefaultSyntaxValidObject()
    {
        return STYLE->standardPalette().link();
    }

    QVariant getDefaultSyntaxForeground()
    {
        return STYLE->standardPalette().text();
    }

    QVariant getDefaultSyntaxStringFg()
    {
        return STYLE->extendedPalette().editorString();
    }

    QVariant getDefaultSyntaxKeywordFg()
    {
        return STYLE->standardPalette().windowText();
    }

    QVariant getDefaultSyntaxBindParamFg()
    {
        return STYLE->standardPalette().linkVisited();
    }

    QVariant getDefaultSyntaxBlobFg()
    {
        return STYLE->standardPalette().text();
    }

    QVariant getDefaultSyntaxCommentFg()
    {
        return STYLE->standardPalette().dark();
    }

    QVariant getDefaultSyntaxNumberFg()
    {
        return STYLE->standardPalette().text();
    }

    DEFINE_COLOR_HELPER_FN(SyntaxParenthesisBg)
    DEFINE_COLOR_HELPER_FN(SyntaxParenthesisFg)
    DEFINE_COLOR_HELPER_FN(SyntaxCurrentLineBg)
    DEFINE_COLOR_HELPER_FN(SyntaxCurrentQueryBg)
    DEFINE_COLOR_HELPER_FN(SyntaxValidObject)
    DEFINE_COLOR_HELPER_FN(SyntaxForeground)
    DEFINE_COLOR_HELPER_FN(SyntaxStringFg)
    DEFINE_COLOR_HELPER_FN(SyntaxKeywordFg)
    DEFINE_COLOR_HELPER_FN(SyntaxBindParamFg)
    DEFINE_COLOR_HELPER_FN(SyntaxBlobFg)
    DEFINE_COLOR_HELPER_FN(SyntaxCommentFg)
    DEFINE_COLOR_HELPER_FN(SyntaxNumberFg)
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
