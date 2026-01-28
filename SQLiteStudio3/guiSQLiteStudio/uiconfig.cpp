#include "uiconfig.h"
#include "style.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QStyle>
#include <QStandardItem>
#include <QDir>
#include <QDebug>

#define DEFINE_FORMAT_FN_BG_FG(FN_NAME, BG_COLOR_NAME, FG_COLOR_NAME) \
    QTextCharFormat FN_NAME() \
    { \
        QTextCharFormat format; \
        format.setForeground(CFG_UI.Colors.FG_COLOR_NAME.get()); \
        format.setBackground(CFG_UI.Colors.BG_COLOR_NAME##Custom.get() ? CFG_UI.Colors.BG_COLOR_NAME.get() : CFG_UI.Colors.BG_COLOR_NAME.getDefaultValue()); \
        format.setFontWeight((CFG_UI.Colors.FG_COLOR_NAME##Custom.get() ? CFG_UI.Colors.FG_COLOR_NAME##Bold.get() : CFG_UI.Colors.FG_COLOR_NAME##Bold.getDefaultValue()) ? QFont::ExtraBold : QFont::Normal); \
        format.setFontItalic(CFG_UI.Colors.FG_COLOR_NAME##Custom.get() ? CFG_UI.Colors.FG_COLOR_NAME##Italic.get() : CFG_UI.Colors.FG_COLOR_NAME##Italic.getDefaultValue()); \
        return format; \
    }

#define DEFINE_FORMAT_FN_FG(FN_NAME, FG_COLOR_NAME) \
    QTextCharFormat FN_NAME() \
    { \
        QTextCharFormat format; \
        format.setForeground(CFG_UI.Colors.FG_COLOR_NAME.get()); \
        format.setFontWeight((CFG_UI.Colors.FG_COLOR_NAME##Custom.get() ? CFG_UI.Colors.FG_COLOR_NAME##Bold.get() : CFG_UI.Colors.FG_COLOR_NAME##Bold.getDefaultValue()) ? QFont::ExtraBold : QFont::Normal); \
        format.setFontItalic(CFG_UI.Colors.FG_COLOR_NAME##Custom.get() ? CFG_UI.Colors.FG_COLOR_NAME##Italic.get() : CFG_UI.Colors.FG_COLOR_NAME##Italic.getDefaultValue()); \
        return format; \
    }

#define DEFINE_FORMAT_FN_BG(FN_NAME, BG_COLOR_NAME) \
    QTextCharFormat FN_NAME() \
    { \
        QTextCharFormat format; \
        format.setBackground(CFG_UI.Colors.BG_COLOR_NAME##Custom.get() ? CFG_UI.Colors.BG_COLOR_NAME.get() : CFG_UI.Colors.BG_COLOR_NAME.getDefaultValue()); \
        return format; \
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

    DEFINE_FORMAT_FN_BG_FG(getSyntaxParenthesisFormat, SyntaxParenthesisBg, SyntaxParenthesisFg)
    DEFINE_FORMAT_FN_BG(getSyntaxCurrentLineFormat, SyntaxCurrentLineBg)
    DEFINE_FORMAT_FN_BG(getSyntaxCurrentQueryFormat, SyntaxCurrentQueryBg)
    DEFINE_FORMAT_FN_FG(getSyntaxValidObjectFormat, SyntaxValidObject)
    DEFINE_FORMAT_FN_FG(getSyntaxForegroundFormat, SyntaxForeground)
    DEFINE_FORMAT_FN_FG(getSyntaxStringFormat, SyntaxStringFg)
    DEFINE_FORMAT_FN_FG(getSyntaxKeywordFormat, SyntaxKeywordFg)
    DEFINE_FORMAT_FN_FG(getSyntaxBindParamFormat, SyntaxBindParamFg)
    DEFINE_FORMAT_FN_FG(getSyntaxBlobFormat, SyntaxBlobFg)
    DEFINE_FORMAT_FN_FG(getSyntaxCommentFormat, SyntaxCommentFg)
    DEFINE_FORMAT_FN_FG(getSyntaxNumberFormat, SyntaxNumberFg)
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
