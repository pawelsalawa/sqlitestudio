#include "uiutils.h"
#include "services/config.h"
#include "common/widgetstateindicator.h"
#include <QObject>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QStringList>
#include <QSet>
#include <QDebug>

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

void setValidState(QWidget *widget, bool valid, const QString& message)
{
    INDICATOR(widget)->setMode(WidgetStateIndicator::Mode::ERROR);
    INDICATOR(widget)->setVisible(!valid, valid ? QString() : message);
}


void setValidStateWihtTooltip(QWidget* widget, const QString& tooltip, bool valid, const QString& message)
{
    if (!valid)
    {
        INDICATOR(widget)->setMode(WidgetStateIndicator::Mode::ERROR);
        INDICATOR(widget)->setVisible(true, message);
    }
    else
    {
        INDICATOR(widget)->setMode(WidgetStateIndicator::Mode::HINT);
        INDICATOR(widget)->setVisible(widget->isEnabled(), tooltip);
    }
}
