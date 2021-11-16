#include "uiutils.h"
#include "services/config.h"
#include "common/widgetstateindicator.h"
#include "common/utils.h"
#include "uiconfig.h"
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
#include <QDesktopWidget>
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QToolBar>
#include <QToolButton>

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

    QStringList filters({
        QObject::tr("All SQLite databases")+" (*.db *.sdb *.sqlite *.db3 *.s3db *.sqlite3 *.sl3)",
        "SQLite3 (*.db3 *.s3db *.sqlite3 *.sl3)",
        QObject::tr("All files")+" (*)"
    });

    QFileDialog dialog(nullptr, QObject::tr("Select database file"), dir, QString());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Select"));
    dialog.setLabelText(QFileDialog::FileType, QObject::tr("File type"));
    dialog.setNameFilters(filters);
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().empty())
        return QString();

    return dialog.selectedFiles().constFirst();
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
        return QString();
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

void limitDialogWidth(QDialog* dialog)
{
    dialog->setMaximumWidth(QGuiApplication::primaryScreen()->availableGeometry().width());
}

void fixTextCursorSelectedText(QString& text)
{
    text.replace("\u2029", "\n");
}

QColor styleSyntaxStringColor()
{
    static const QColor stdAltColor = QColor(Qt::green);
    if (QApplication::style()->standardPalette().text().color().lightness() >= 128)
        return stdAltColor.lighter();
    else
        return stdAltColor.darker();
}

QBrush styleEditorLineColor()
{
    QPalette palette = QApplication::style()->standardPalette();
    if (CFG_UI.General.Style.get().toLower() != "macintosh")
        return palette.alternateBase();

    if (palette.base().color().lightness() < 128)
        return QBrush(palette.alternateBase().color().darker(300));

    return palette.alternateBase();
}

void fixToolbarTooltips(QToolBar* toolbar)
{
    for (QAction*& action : toolbar->actions())
    {
        QToolButton* button = dynamic_cast<QToolButton*>(toolbar->widgetForAction(action));
        if (!button)
            continue;

        QString text = action->text();
        text = text.replace(QRegExp("\\s?\\(&.\\)$"),""); // issue #4261, for Chinese
        text = text.replace("&", ""); // issue #4218
        if (!action->shortcut().isEmpty())
            text += QString(" (%1)").arg(action->shortcut().toString());

        button->setToolTip(text);
    }
}
