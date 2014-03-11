#include "uiutils.h"
#include "config.h"
#include <QObject>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QStringList>

QString getDbPath(const QString &startWith)
{
    QString dir = startWith;
    if (dir.isNull())
        dir = CFG->get("dialogCache", "lastDbDir").toString();

    QStringList filters;
    filters += QObject::tr("All SQLite databases")+" (*.db *.sdb *.sqlite *.db3 *.s3db *.sqlite3 *.sl3 *.db2 *.s2db *.sqlite2 *.sl2)";
    filters += "SQLite3 (*.db3 *.s3db *.sqlite3 *.sl3)";
    filters += "SQLite2 (*.db2 *.s2db *.sqlite2 *.sl2)";
    filters += QObject::tr("All files")+" (*)";
    QString filter = filters.join(";;");

    QString path = QFileDialog::getSaveFileName(0, QObject::tr("Database file"), dir, filter, &filters[0], QFileDialog::DontConfirmOverwrite);

    if (!path.isNull())
        CFG->set("dialogCache", "lastDbDir", QFileInfo(path).dir().absolutePath());

    return path;
}

bool isBgTypeOfInvalidWidget(QWidget* widget)
{
    return (dynamic_cast<QLineEdit*>(widget) != nullptr);
}

void setValidStyle(QWidget *widget, bool valid)
{
    static const QString invalidStyleStr = QStringLiteral("%1 {color: red}");
    static const QString invalidBgStyleStr = QStringLiteral("%1 {background: red}");

    if (isBgTypeOfInvalidWidget(widget))
        widget->setStyleSheet(valid ? "" : invalidBgStyleStr.arg(widget->metaObject()->className()));
    else
        widget->setStyleSheet(valid ? "" : invalidStyleStr.arg(widget->metaObject()->className()));
}
