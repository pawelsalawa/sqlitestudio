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

const QList<QPageSize::PageSizeId> pageSizeIds = {
    QPageSize::A4, QPageSize::B5, QPageSize::Letter, QPageSize::Legal, QPageSize::Executive, QPageSize::A0, QPageSize::A1,
    QPageSize::A2, QPageSize::A3, QPageSize::A5, QPageSize::A6, QPageSize::A7, QPageSize::A8, QPageSize::A9, QPageSize::B0,
    QPageSize::B1, QPageSize::B10, QPageSize::B2, QPageSize::B3, QPageSize::B4, QPageSize::B6, QPageSize::B7, QPageSize::B8,
    QPageSize::B9, QPageSize::C5E, QPageSize::Comm10E, QPageSize::DLE, QPageSize::Folio, QPageSize::Ledger, QPageSize::Tabloid,
    QPageSize::Custom
};

const QStringList pageSizes = map<QPageSize::PageSizeId, QString>(pageSizeIds, [](QPageSize::PageSizeId id) -> QString
{
    return QPageSize::name(id);
});

const QStringList pageSizesWithDimensions;

QString getDbPath(bool newFileMode, const QString &startWith)
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
    dialog.setAcceptMode(newFileMode ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);

    /* As we don't actually overwrite a selected existing database file, switch off the
     * overwrite warning.
     * FIXME: QFileDialog::DontConfirmOverwrite does not work on MacOS native dialogs.
     * Probably some better UX is needed.
     */
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

QString convertPageSize(QPageSize::PageSizeId size)
{
    return QPageSize::name(size);
}

QPageSize convertPageSize(const QString& size)
{
    return QPageSize(static_cast<QPageSize::PageSizeId>(indexOf(pageSizes, size, Qt::CaseInsensitive)));
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
