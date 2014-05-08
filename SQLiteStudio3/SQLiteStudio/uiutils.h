#ifndef UIUTILS_H
#define UIUTILS_H

#include <QVariant>
#include <QWidget>

QString getDbPath(const QString& startWith = QString::null);
void setValidState(QWidget* widget, bool valid, const QString& message = QString());
void setValidStateWihtTooltip(QWidget* widget, const QString& tooltip, bool valid, const QString& message = QString());

#endif // UIUTILS_H
