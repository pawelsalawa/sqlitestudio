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
    enum InsertRowPlacement
    {
        BEFORE_CURRENT,
        AFTER_CURRENT,
        AT_THE_END
    };
}

CFG_CATEGORIES(Ui,
    CFG_CATEGORY(Fonts,
        CFG_ENTRY(QFont,        SqlEditor,               &Cfg::getDefaultTextEditorFont)
        CFG_ENTRY(QFont,        DataView,                &Cfg::getDefaultItemViewFont)
        CFG_ENTRY(QFont,        DbTree,                  &Cfg::getDefaultItemViewFont)
        CFG_ENTRY(QFont,        DbTreeLabel,             &Cfg::getDefaultDbTreeLabelFont)
        CFG_ENTRY(QFont,        StatusField,             &Cfg::getDefaultItemViewFont)
    )

    CFG_CATEGORY(DbList,
    )

    CFG_CATEGORY(General,
        CFG_ENTRY(QString,               DataViewTabs,               QString())
        CFG_ENTRY(QString,               SqlEditorTabs,              QString())
        CFG_ENTRY(QString,               SqlEditorDbListOrder,       "LikeDbTree")
        CFG_ENTRY(bool,                  ExpandTables,               true)
        CFG_ENTRY(bool,                  ExpandViews,                true)
        CFG_ENTRY(bool,                  SortObjects,                true)
        CFG_ENTRY(bool,                  SortColumns,                false)
        CFG_ENTRY(bool,                  ExecuteCurrentQueryOnly,    true)
        CFG_ENTRY(bool,                  ShowSystemObjects,          false)
        CFG_ENTRY(bool,                  ShowDbTreeLabels,           true) // any labels at all
        CFG_ENTRY(bool,                  ShowRegularTableLabels,     false)
        CFG_ENTRY(bool,                  ShowVirtualTableLabels,     true)
        CFG_ENTRY(int,                   NumberOfRowsPerPage,        1000)
        CFG_ENTRY(QString,               Style,                      &Cfg::getStyleDefaultValue)
        CFG_ENTRY(Cfg::Session,          Session,                    Cfg::Session())
        CFG_ENTRY(bool,                  AllowMultipleSessions,      false)
        CFG_ENTRY(bool,                  RestoreSession,             true)
        CFG_ENTRY(bool,                  DontShowDdlPreview,         false)
        CFG_ENTRY(bool,                  OpenTablesOnData,           false)
        CFG_ENTRY(bool,                  DataTabAsFirstInTables,     false)
        CFG_ENTRY(bool,                  OpenViewsOnData,            false)
        CFG_ENTRY(bool,                  DataTabAsFirstInViews,      false)
        CFG_ENTRY(bool,                  AutoOpenStatusField,        true)
        CFG_ENTRY(bool,                  NewDbNotPermanentByDefault, false)
        CFG_ENTRY(bool,                  BypassDbDialogWhenDropped,  false)
        CFG_ENTRY(Cfg::DataEditorsOrder, DataEditorsOrder,           Cfg::DataEditorsOrder())
        CFG_ENTRY(QString,               FileDialogLastPath,         QString())
        CFG_ENTRY(int,                   MaxInitialColumnWith,       600)
        CFG_ENTRY(bool,                  LanguageAsked,              false)
        CFG_ENTRY(bool,                  OpenMaximized,              true)
        CFG_ENTRY(QString,               DockLayout,                 "vertical")
        CFG_ENTRY(QString,               CustomCss,                  QString())
        CFG_ENTRY(bool,                  CompactLayout,              true)
        CFG_ENTRY(int,                   InsertRowPlacement,         Cfg::BEFORE_CURRENT)
        CFG_ENTRY(bool,                  ShowDataViewTooltips,       true)
        CFG_ENTRY(bool,                  KeepNullWhenEmptyValue,     true)
        CFG_ENTRY(bool,                  UseDefaultValueForNull,     false)
    )
)

GUI_API_EXPORT QString getFileDialogInitPath();
GUI_API_EXPORT void setFileDialogInitPath(const QString& path);
GUI_API_EXPORT void setFileDialogInitPathByFile(const QString& filePath);

#define CFG_UI CFG_INSTANCE(Ui)

#endif // UICONFIG_H
