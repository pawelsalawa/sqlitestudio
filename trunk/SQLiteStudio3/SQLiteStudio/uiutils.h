#ifndef UIUTILS_H
#define UIUTILS_H

#include <QVariant>
#include <QPagedPaintDevice>
#include "uiapiexport.h"

class QWidget;

QString getDbPath(const QString& startWith = QString::null);
void setValidState(QWidget* widget, bool valid, const QString& message = QString());
void setValidStateWihtTooltip(QWidget* widget, const QString& tooltip, bool valid, const QString& message = QString());
void setValidStateWarning(QWidget* widget, const QString& warning);
UI_API_EXPORT const QStringList& getAllPageSizes();
UI_API_EXPORT QString convertPageSize(QPagedPaintDevice::PageSize size);
UI_API_EXPORT QPagedPaintDevice::PageSize convertPageSize(const QString& size);

#endif // UIUTILS_H
