#ifndef UICONFIG_H
#define UICONFIG_H

#include "cfginternals.h"
#include <QFont>
#include <QHash>
#include <QColor>

namespace Cfg
{
    QVariant getStyleDefaultValue();
    QVariant getDefaultTextEditorFont();
    QVariant getDefaultItemViewFont();
    QVariant getDefaultDbTreeLabelFont();
    typedef QHash<QString,QVariant> Session;
}

CFG_CATEGORIES(Ui,
    CFG_CATEGORY(Fonts,
        CFG_ENTRY(QFont,        SqlEditor,               "SqlEditor",                      &Cfg::getDefaultTextEditorFont)
        CFG_ENTRY(QFont,        DataView,                "DataView",                       &Cfg::getDefaultItemViewFont)
        CFG_ENTRY(QFont,        DbTree,                  "DbTree",                         &Cfg::getDefaultItemViewFont)
        CFG_ENTRY(QFont,        DbTreeLabel,             "DbTreeLabel",                    &Cfg::getDefaultDbTreeLabelFont)
        CFG_ENTRY(QFont,        StatusField,             "StatusField",                    &Cfg::getDefaultItemViewFont)
    )

    CFG_CATEGORY(Colors,
        CFG_ENTRY(QColor,       SqlEditorParenthesisBg,  "SqlEditorParenthesisBackground", Qt::green)
        CFG_ENTRY(QColor,       SqlEditorCurrentLineBg,  "SqlEditorCurrentLineBackground", QColor(Qt::cyan).lighter(190))
        CFG_ENTRY(QColor,       SqlEditorLineNumAreaBg,  "SqlEditorLineNumAreaBackground", QColor(Qt::lightGray).lighter(120))
        CFG_ENTRY(QColor,       SqlEditorValidObject,    "SqlEditorValidObject",           Qt::blue)
        CFG_ENTRY(QColor,       SqlEditorForeground,     "SqlEditorForeground",            Qt::black)
        CFG_ENTRY(QColor,       SqlEditorStringFg,       "SqlEditorStringForeground",      Qt::darkGreen)
        CFG_ENTRY(QColor,       SqlEditorKeywordFg,      "SqlEditorKeywordForeground",     Qt::black)
        CFG_ENTRY(QColor,       SqlEditorBindParamFg,    "SqlEditorBindParamForeground",   Qt::darkMagenta)
        CFG_ENTRY(QColor,       SqlEditorBlobFg,         "SqlEditorBlobForeground",        Qt::darkCyan)
        CFG_ENTRY(QColor,       SqlEditorCommentFg,      "SqlEditorCommentForeground",     Qt::darkGray)
        CFG_ENTRY(QColor,       SqlEditorNumberFg,       "SqlEditorNumberForeground",      Qt::darkBlue)
        CFG_ENTRY(QColor,       DataUncommitedError,     "DataUncommitedError",            Qt::red)
        CFG_ENTRY(QColor,       DataUncommited,          "DataUncommited",                 Qt::blue)
        CFG_ENTRY(QColor,       DataNullFg,              "DataNullForeground",             Qt::gray)
        CFG_ENTRY(QColor,       DataDeletedBg,           "DataDeletedBackground",          Qt::gray)
        CFG_ENTRY(QColor,       DbTreeLabelsFg,          "DbTreeLabelsForeground",         Qt::blue)
        CFG_ENTRY(QColor,       StatusFieldInfoFg,       "StatusFieldInfoForeground",      Qt::darkBlue)
        CFG_ENTRY(QColor,       StatusFieldWarnFg,       "StatusFieldWarnForeground",      Qt::black)
        CFG_ENTRY(QColor,       StatusFieldErrorFg,      "StatusFieldErrorForeground",     Qt::red)
    )

    CFG_CATEGORY(General,
        CFG_ENTRY(QString,      DataViewTabs,            "DataViewTabs",                   QString())
        CFG_ENTRY(QString,      SqlEditorTabs,           "SqlEditorTabs",                  QString())
        CFG_ENTRY(QString,      SqlEditorDbListOrder,    "SqlEditorDbListOrder",           "LikeDbTree")
        CFG_ENTRY(bool,         ExpandTables,            "ExpandDbTreeTables",             true)
        CFG_ENTRY(bool,         ExpandViews,             "ExpandDbTreeViews",              true)
        CFG_ENTRY(bool,         SortObjects,             "SortObjectsInDbTree",            true)
        CFG_ENTRY(bool,         SortColumns,             "SortColumnsInDbTree",            false)
        CFG_ENTRY(bool,         ExecuteCurrentQueryOnly, "ExecuteCurrentQueryOnly",        false)
        CFG_ENTRY(bool,         ShowSystemObjects,       "ShowSystemObjectsInDbTree",      false)
        CFG_ENTRY(bool,         ShowDbTreeLabels,        "ShowLabelsInDbTree",             true) // any labels at all
        CFG_ENTRY(bool,         ShowRegularTableLabels,  "ShowRegularTableLabelsInDbTree", false)
        CFG_ENTRY(int,          NumberOfRowsPerPage,     "NumberOfDataRowsPerPage",        1000)
        CFG_ENTRY(QString,      Style,                   "Style",                          &Cfg::getStyleDefaultValue)
        CFG_ENTRY(Cfg::Session, Session,                 "session",                        Cfg::Session())
        CFG_ENTRY(bool,         DontShowDdlPreview,      "DontShowDdlPreview",             false)
        CFG_ENTRY(bool,         OpenTablesOnData,        "OpenTablesOnData",               false)
        CFG_ENTRY(bool,         OpenViewsOnData,         "OpenViewsOnData",                false)
    )
)

CFG_DECLARE(Ui)
#define CFG_UI CFG_INSTANCE(Ui)

#endif // UICONFIG_H
