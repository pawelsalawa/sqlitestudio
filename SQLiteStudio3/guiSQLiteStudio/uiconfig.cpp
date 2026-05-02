#include "uiconfig.h"
#include "style.h"
#include "common/global.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QStyle>
#include <QStandardItem>
#include <QDir>
#include <QDebug>
#include <QSplitter>
#include <QTimer>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtSystemDetection>
#else
#include <qsystemdetection.h>
#endif

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
        return STYLE->standardPalette().windowText().color();
    }

    QVariant getDefaultSyntaxParenthesisFg()
    {
        return STYLE->standardPalette().window().color();
    }

    QVariant getDefaultSyntaxCurrentLineBg()
    {
        return STYLE->extendedPalette().editorLineBase().color();
    }

    QVariant getDefaultSyntaxCurrentQueryBg()
    {
        return STYLE->extendedPalette().editorCurrentQueryBase().color();
    }

    QVariant getDefaultSyntaxValidObject()
    {
        return STYLE->standardPalette().link().color();
    }

    QVariant getDefaultSyntaxForeground()
    {
        return STYLE->standardPalette().text().color();
    }

    QVariant getDefaultSyntaxStringFg()
    {
        return STYLE->extendedPalette().editorString().color();
    }

    QVariant getDefaultSyntaxKeywordFg()
    {
        return STYLE->standardPalette().windowText().color();
    }

    QVariant getDefaultSyntaxBindParamFg()
    {
        return STYLE->standardPalette().linkVisited().color();
    }

    QVariant getDefaultSyntaxBlobFg()
    {
        return STYLE->standardPalette().text().color();
    }

    QVariant getDefaultSyntaxCommentFg()
    {
        return STYLE->extendedPalette().editorComment().color();
    }

    QVariant getDefaultSyntaxNumberFg()
    {
        return STYLE->standardPalette().text().color();
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

    void handleSplitterState(QSplitter* splitter)
    {
        static_qstring(keyTpl, "%1.%2");
        QString key = keyTpl.arg(splitter->window()->objectName(), splitter->objectName());
        QHash<QString, QByteArray> savedStates = CFG_UI.General.SplitterStates.get();
        if (savedStates.contains(key))
            splitter->restoreState(savedStates[key]);

        QTimer* timer = new QTimer(splitter);
        timer->setSingleShot(true);
        timer->setInterval(500);

        QObject::connect(splitter, SIGNAL(splitterMoved(int,int)), timer, SLOT(start()));
        QObject::connect(splitter, SIGNAL(destroyed(QObject*)), timer, SLOT(stop()));
        QObject::connect(timer, &QTimer::timeout, [key, splitter]()
        {
            QHash<QString, QByteArray> savedStates = CFG_UI.General.SplitterStates.get();
            savedStates[key] = splitter->saveState();
            CFG_UI.General.SplitterStates.set(savedStates);
        });
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
