#ifndef UIUTILS_H
#define UIUTILS_H

#include <QVariant>
#include <QWidget>

class QTextCodec;

QString getDbPath(const QString& startWith = QString::null);
void setValidStyle(QWidget* widget, bool valid);
QStringList textCodecNames();
QString defaultCodecName();
QTextCodec* codecForName(const QString& name);

#endif // UIUTILS_H
