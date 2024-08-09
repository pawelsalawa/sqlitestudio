#include "multieditordatetime.h"
#include "common/utils.h"
#include "common/unused.h"
#include <QDateTimeEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QCalendarWidget>
#include <QWheelEvent>
#include <QTableView>
#include <QLabel>

QStringList MultiEditorDateTime::formats;

MultiEditorDateTime::MultiEditorDateTime(QWidget *parent) :
    MultiEditorWidget(parent)
{
    QVBoxLayout* vbox = new QVBoxLayout();
    setLayout(vbox);
    dateTimeEdit = new QDateTimeEdit();
    dateTimeLabel = new QLabel();
    calendar = new QCalendarWidget();
    // Extending width, becuase day labels are truncated on some systems.
    calendar->setFixedSize(calendar->sizeHint() + QSize(80, 0));

    vbox->addWidget(dateTimeEdit);
    vbox->addWidget(dateTimeLabel);
    vbox->addWidget(calendar);

    setDisplayFormat(formats.first());

    connect(calendar, &QCalendarWidget::selectionChanged, this, &MultiEditorDateTime::calendarDateChanged);
    connect(dateTimeEdit, &QDateTimeEdit::dateChanged, this, &MultiEditorDateTime::dateChanged);
    connect(dateTimeEdit, &QDateTimeEdit::timeChanged, this, &MultiEditorDateTime::timeChanged);

    setFocusProxy(dateTimeEdit);
    updateCalendarDisplay();
}

void MultiEditorDateTime::staticInit()
{
    formats << "yyyy-MM-dd hh:mm:ss"
            << "yyyy-MM-dd hh:mm"
            << "yyyy-MM-dd"
            << "yyyy-MM-dd hh:mm:ss.z"
            << "yyyy-MM-ddThh:mm"
            << "yyyy-MM-ddThh:mm:ss"
            << "yyyy-MM-ddThh:mm:ss.z";
}

void MultiEditorDateTime::setDisplayFormat(const QString& format)
{
    dateTimeEdit->setDisplayFormat(format);
    dateTimeEdit->setMaximumWidth(dateTimeEdit->sizeHint().width());
}

void MultiEditorDateTime::setValue(const QVariant& value)
{
    switch (value.userType())
    {
        case QVariant::DateTime:
            dateTimeEdit->setDateTime(value.toDateTime());
            break;
        case QVariant::Date:
            dateTimeEdit->setDate(value.toDate());
            break;
        default:
        {
            dateTimeEdit->setDateTime(fromString(value.toString()));
            break;
        }
    }
    updateReadOnlyDisplay();
}

QVariant MultiEditorDateTime::getValue()
{
    if (formatType == STRING)
        return dateTimeEdit->dateTime().toString(originalValueFormat);
    else if (formatType == UNIXTIME)
        return dateTimeEdit->dateTime().toSecsSinceEpoch();
    else if (formatType == JULIAN_DAY)
        return toJulian(dateTimeEdit->dateTime());
    else
        return dateTimeEdit->dateTime().toString(dateTimeEdit->displayFormat());
}

QList<QWidget*> MultiEditorDateTime::getNoScrollWidgets()
{
    QList<QWidget*> list;
    list << dateTimeEdit << calendar;

    QObject* obj = calendar->findChild<QTableView*>("qt_calendar_calendarview");
    if (obj)
    {
        QTableView* view = dynamic_cast<QTableView*>(obj);
        if (view)
            list << view->viewport();
    }

    return list;
}

QDateTime MultiEditorDateTime::fromString(const QString& value)
{
    QDateTime dateTime;
    for (const QString& format : getParsingFormats())
    {
        dateTime = QDateTime::fromString(value, format);
        if (dateTime.isValid())
        {
            formatType = STRING;
            originalValueFormat = format;
            return dateTime;
        }
    }

    // Try with unixtime
    bool ok;
    uint unixtime = value.toUInt(&ok);
    if (ok)
    {
        dateTime = QDateTime::fromSecsSinceEpoch(unixtime);
        formatType = UNIXTIME;
        return dateTime;
    }

    // Try with Julian day
    double jd = value.toDouble(&ok);
    if (ok)
    {
        dateTime = toGregorian(jd);
        formatType = JULIAN_DAY;
        return dateTime;
    }

    formatType = OTHER;
    return QDateTime();
}

void MultiEditorDateTime::calendarDateChanged()
{
    if (updatingCalendar)
        return;

    dateTimeEdit->setDate(calendar->selectedDate());
    emit valueModified();
}

void MultiEditorDateTime::dateChanged(const QDate& date)
{
    updatingCalendar = true;
    calendar->setSelectedDate(date);
    updatingCalendar = false;
    emit valueModified();
}

void MultiEditorDateTime::timeChanged(const QTime& time)
{
    UNUSED(time);
    emit valueModified();
}

bool MultiEditorDateTime::getReadOnly() const
{
    return readOnly;
}

void MultiEditorDateTime::setReadOnly(bool value)
{
    readOnly = value;
    dateTimeEdit->setVisible(!readOnly);
    dateTimeLabel->setVisible(readOnly);
    updateReadOnlyDisplay();
}

void MultiEditorDateTime::focusThisWidget()
{
    dateTimeEdit->setFocus();
}

QStringList MultiEditorDateTime::getParsingFormats()
{
    return formats;
}

void MultiEditorDateTime::updateReadOnlyDisplay()
{
    if (!readOnly)
        return;

    dateTimeLabel->setText(getValue().toString());
    QDate date = dateTimeEdit->date();
    calendar->setMinimumDate(date);
    calendar->setMaximumDate(date);
    calendar->setSelectedDate(date);
}

void MultiEditorDateTime::updateCalendarDisplay()
{
    if (!showCalendars)
    {
        calendar->setVisible(false);
        return;
    }
}

MultiEditorWidget*MultiEditorDateTimePlugin::getInstance()
{
    return new MultiEditorDateTime();
}

bool MultiEditorDateTimePlugin::validFor(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::NONE:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::TIME:
        case DataType::ANY:
        case DataType::unknown:
            break;
        case DataType::DATE:
        case DataType::DATETIME:
            return true;
    }
    return false;
}

int MultiEditorDateTimePlugin::getPriority(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::NONE:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::TIME:
        case DataType::ANY:
        case DataType::unknown:
            break;
        case DataType::DATE:
            return 2;
        case DataType::DATETIME:
            return 1;
    }
    return 10;
}

QString MultiEditorDateTimePlugin::getTabLabel()
{
    return tr("Date & time");
}
