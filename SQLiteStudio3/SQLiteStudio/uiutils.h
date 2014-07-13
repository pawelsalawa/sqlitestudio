#ifndef UIUTILS_H
#define UIUTILS_H

#include <QVariant>
#include <QPagedPaintDevice>

class QWidget;

QString getDbPath(const QString& startWith = QString::null);
void setValidState(QWidget* widget, bool valid, const QString& message = QString());
void setValidStateWihtTooltip(QWidget* widget, const QString& tooltip, bool valid, const QString& message = QString());
void setValidStateWarning(QWidget* widget, const QString& warning);
void setValidStateInfo(QWidget* widget, const QString& info);
void setValidStateTooltip(QWidget* widget, const QString& tip);
const QStringList& getAllPageSizes();
QString convertPageSize(QPagedPaintDevice::PageSize size);
QPagedPaintDevice::PageSize convertPageSize(const QString& size);

#endif // UIUTILS_H
