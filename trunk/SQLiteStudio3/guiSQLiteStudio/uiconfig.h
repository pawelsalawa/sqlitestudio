#ifndef UICONFIG_H
#define UICONFIG_H

#include "guiSQLiteStudio_global.h"
#include "config_builder.h"
#include <QFont>
#include <QHash>
#include <QColor>

namespace Cfg
{
    GUI_API_EXPORT QVariant getStyleDefaultValue();
    GUI_API_EXPORT QVariant getDefaultTextEditorFont();
    GUI_API_EXPORT QVariant getDefaultItemViewFont();
    GUI_API_EXPORT QVariant getDefaultDbTreeLabelFont();
    typedef QHash<QString,QVariant> Session;
    typedef QHash<QString,QVariant> DataEditorsOrder;
}

CFG_CATEGORIES(Ui,
    CFG_CATEGORY(Fonts,
        CFG_ENTRY(QFont,        SqlEditor,               &Cfg::getDefaultTextEditorFont)
        CFG_ENTRY(QFont,        DataView,                &Cfg::getDefaultItemViewFont)
        CFG_ENTRY(QFont,        DbTree,                  &Cfg::getDefaultItemViewFont)
        CFG_ENTRY(QFont,        DbTreeLabel,             &Cfg::getDefaultDbTreeLabelFont)
        CFG_ENTRY(QFont,        StatusField,             &Cfg::getDefaultItemViewFont)
    )

    CFG_CATEGORY(Colors,
        CFG_ENTRY(QColor,       SqlEditorParenthesisBg,  Qt::green)
        CFG_ENTRY(QColor,       SqlEditorCurrentLineBg,  QColor(Qt::cyan).lighter(190))
        CFG_ENTRY(QColor,       SqlEditorLineNumAreaBg,  QColor(Qt::lightGray).lighter(120))
        CFG_ENTRY(QColor,       SqlEditorValidObject,    Qt::blue)
        CFG_ENTRY(QColor,       SqlEditorForeground,     Qt::black)
        CFG_ENTRY(QColor,       SqlEditorStringFg,       Qt::darkGreen)
        CFG_ENTRY(QColor,       SqlEditorKeywordFg,      Qt::black)
        CFG_ENTRY(QColor,       SqlEditorBindParamFg,    Qt::darkMagenta)
        CFG_ENTRY(QColor,       SqlEditorBlobFg,         Qt::darkCyan)
        CFG_ENTRY(QColor,       SqlEditorCommentFg,      Qt::darkGray)
        CFG_ENTRY(QColor,       SqlEditorNumberFg,       Qt::darkBlue)
        CFG_ENTRY(QColor,       DataUncommitedError,     Qt::red)
        CFG_ENTRY(QColor,       DataUncommited,          Qt::blue)
        CFG_ENTRY(QColor,       DataNullFg,              Qt::gray)
        CFG_ENTRY(QColor,       DataDeletedBg,           Qt::gray)
        CFG_ENTRY(QColor,       DbTreeLabelsFg,          Qt::blue)
        CFG_ENTRY(QColor,       StatusFieldInfoFg,       Qt::darkBlue)
        CFG_ENTRY(QColor,       StatusFieldWarnFg,       Qt::black)
        CFG_ENTRY(QColor,       StatusFieldErrorFg,      Qt::red)
        CFG_ENTRY(QColor,       JavaScriptFg,            "#000000")
        CFG_ENTRY(QColor,       JavaScriptComment,       "#808080")
        CFG_ENTRY(QColor,       JavaScriptNumber,        "#008000")
        CFG_ENTRY(QColor,       JavaScriptString,        "#800000")
        CFG_ENTRY(QColor,       JavaScriptOperator,      "#808000")
        CFG_ENTRY(QColor,       JavaScriptIdentifier,    "#000020")
        CFG_ENTRY(QColor,       JavaScriptKeyword,       "#000080")
        CFG_ENTRY(QColor,       JavaScriptBuiltIn,       "#008080")
        CFG_ENTRY(QColor,       JavaScriptMarker,        "#ffff00")
    )

    CFG_CATEGORY(General,
        CFG_ENTRY(QString,               DataViewTabs,            QString())
        CFG_ENTRY(QString,               SqlEditorTabs,           QString())
        CFG_ENTRY(QString,               SqlEditorDbListOrder,    "LikeDbTree")
        CFG_ENTRY(bool,                  ExpandTables,            true)
        CFG_ENTRY(bool,                  ExpandViews,             true)
        CFG_ENTRY(bool,                  SortObjects,             true)
        CFG_ENTRY(bool,                  SortColumns,             false)
        CFG_ENTRY(bool,                  ExecuteCurrentQueryOnly, true)
        CFG_ENTRY(bool,                  ShowSystemObjects,       false)
        CFG_ENTRY(bool,                  ShowDbTreeLabels,        true) // any labels at all
        CFG_ENTRY(bool,                  ShowRegularTableLabels,  false)
        CFG_ENTRY(bool,                  ShowVirtualTableLabels,  true)
        CFG_ENTRY(int,                   NumberOfRowsPerPage,     1000)
        CFG_ENTRY(QString,               Style,                   &Cfg::getStyleDefaultValue)
        CFG_ENTRY(Cfg::Session,          Session,                 Cfg::Session())
        CFG_ENTRY(bool,                  DontShowDdlPreview,      false)
        CFG_ENTRY(bool,                  OpenTablesOnData,        false)
        CFG_ENTRY(bool,                  OpenViewsOnData,         false)
        CFG_ENTRY(Cfg::DataEditorsOrder, DataEditorsOrder,        Cfg::DataEditorsOrder())
        CFG_ENTRY(QString,               FileDialogLastPath,      QString())
    )
)

QString getFileDialogInitPath();
void setFileDialogInitPath(const QString& path);
void setFileDialogInitPathByFile(const QString& filePath);

#define CFG_UI CFG_INSTANCE(Ui)

#endif // UICONFIG_H
