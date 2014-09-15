#include "uiutils.h"
#include "services/config.h"
#include "common/widgetstateindicator.h"
#include "common/utils.h"
#include <QObject>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QStringList>
#include <QSet>
#include <QDebug>
#include <QPainter>

const QStringList pageSizes = {
    "A4", "B5", "Letter", "Legal", "Executive", "A0", "A1", "A2", "A3", "A5", "A6", "A7", "A8", "A9", "B0", "B1",
    "B10", "B2", "B3", "B4", "B6", "B7", "B8", "B9", "C5E", "Comm10E", "DLE", "Folio", "Ledger", "Tabloid", "Custom"
};

const QStringList pageSizesWithDimensions;

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

void setValidStateWarning(QWidget* widget, const QString& warning)
{
    INDICATOR(widget)->setMode(WidgetStateIndicator::Mode::WARNING);
    INDICATOR(widget)->setVisible(widget->isEnabled(), warning);
}

void setValidStateInfo(QWidget* widget, const QString& info)
{
    INDICATOR(widget)->setMode(WidgetStateIndicator::Mode::INFO);
    INDICATOR(widget)->setVisible(widget->isEnabled(), info);
}

void setValidStateTooltip(QWidget* widget, const QString& tip)
{
    INDICATOR(widget)->setMode(WidgetStateIndicator::Mode::HINT);
    INDICATOR(widget)->setVisible(widget->isEnabled(), tip);
}

QString convertPageSize(QPagedPaintDevice::PageSize size)
{
    const int pageSizesSize = pageSizes.size();

    int idx = static_cast<int>(size);
    if (idx < 0 || idx >= pageSizesSize)
    {
        qDebug() << "Asked to convertPageSize() with page side enum value out of range:" << idx;
        return QString::null;
    }

    return pageSizes[idx];
}

QPagedPaintDevice::PageSize convertPageSize(const QString& size)
{
    return static_cast<QPagedPaintDevice::PageSize>(indexOf(pageSizes, size, Qt::CaseInsensitive));
}

const QStringList& getAllPageSizes()
{
    return pageSizes;
}

QPixmap addOpacity(const QPixmap& input, float opacity)
{
    QPixmap output(input.size());
    output.fill(Qt::transparent);
    QPainter p(&output);
    p.setOpacity(opacity);
    p.drawPixmap(0, 0, input);
    p.end();
    return output;
}
