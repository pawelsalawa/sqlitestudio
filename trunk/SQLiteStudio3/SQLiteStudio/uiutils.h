#ifndef UIUTILS_H
#define UIUTILS_H

#include <QVariant>
#include <QWidget>

QString getDbPath(const QString& startWith = QString::null);
void setValidStyle(QWidget* widget, bool valid);

#endif // UIUTILS_H
