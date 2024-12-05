#ifndef UIUTILS_H
#define UIUTILS_H

#include "guiSQLiteStudio_global.h"
#include <QVariant>
#include <QPagedPaintDevice>

class QWidget;
class QToolBar;

GUI_API_EXPORT QString getDbPath(bool newFileMode, const QString& startWith = QString());
GUI_API_EXPORT void setValidState(QWidget* widget, bool valid, const QString& message = QString());
GUI_API_EXPORT void setValidStateWihtTooltip(QWidget* widget, const QString& tooltip, bool valid, const QString& message = QString());
GUI_API_EXPORT void setValidStateWarning(QWidget* widget, const QString& warning);
GUI_API_EXPORT void setValidStateInfo(QWidget* widget, const QString& info);
GUI_API_EXPORT void setValidStateTooltip(QWidget* widget, const QString& tip);
GUI_API_EXPORT bool isValidStateIndicatorVisible(QWidget* widget);
GUI_API_EXPORT const QStringList& getAllPageSizes();
GUI_API_EXPORT QString convertPageSize(QPageSize::PageSizeId size);
GUI_API_EXPORT QPageSize convertPageSize(const QString& size);
GUI_API_EXPORT QPixmap addOpacity(const QPixmap& input, float opacity);
GUI_API_EXPORT void limitDialogWidth(QDialog* dialog);
GUI_API_EXPORT void fixTextCursorSelectedText(QString& text);
GUI_API_EXPORT QColor styleSyntaxStringColor();
GUI_API_EXPORT QBrush styleEditorLineColor();
GUI_API_EXPORT void fixToolbarTooltips(QToolBar* toolbar);

#define UI_PROP_COLUMN "column_name"

#endif // UIUTILS_H
