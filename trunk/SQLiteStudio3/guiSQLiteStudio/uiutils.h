#ifndef UIUTILS_H
#define UIUTILS_H

#include "guiSQLiteStudio_global.h"
#include <QVariant>
#include <QPagedPaintDevice>

class QWidget;

GUI_API_EXPORT QString getDbPath(const QString& startWith = QString::null);
GUI_API_EXPORT void setValidState(QWidget* widget, bool valid, const QString& message = QString());
GUI_API_EXPORT void setValidStateWihtTooltip(QWidget* widget, const QString& tooltip, bool valid, const QString& message = QString());
GUI_API_EXPORT void setValidStateWarning(QWidget* widget, const QString& warning);
GUI_API_EXPORT void setValidStateInfo(QWidget* widget, const QString& info);
GUI_API_EXPORT void setValidStateTooltip(QWidget* widget, const QString& tip);
GUI_API_EXPORT const QStringList& getAllPageSizes();
GUI_API_EXPORT QString convertPageSize(QPagedPaintDevice::PageSize size);
GUI_API_EXPORT QPagedPaintDevice::PageSize convertPageSize(const QString& size);
GUI_API_EXPORT QPixmap addOpacity(const QPixmap& input, float opacity);

#endif // UIUTILS_H
