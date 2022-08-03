#include "multieditortime.h"
#include <QTimeEdit>

QStringList MultiEditorTime::formats;

MultiEditorTime::MultiEditorTime(QWidget *parent)
    : MultiEditorDateTime(parent)
{
    showCalendars = false;
    updateCalendarDisplay();
    setDisplayFormat(formats.first());
}

void MultiEditorTime::staticInit()
{
    formats << "hh:mm:ss"
            << "hh:mm:ss.zzz"
            << "hh:mm";
}

QStringList MultiEditorTime::getParsingFormats()
{
    return formats;
}

MultiEditorWidget*MultiEditorTimePlugin::getInstance()
{
    return new MultiEditorTime();
}

bool MultiEditorTimePlugin::validFor(const DataType& dataType)
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
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::ANY:
        case DataType::unknown:
            break;
        case DataType::TIME:
            return true;
    }
    return false;
}

int MultiEditorTimePlugin::getPriority(const DataType& dataType)
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
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::ANY:
        case DataType::unknown:
            break;
        case DataType::TIME:
            return 1;
    }
    return 10;
}

QString MultiEditorTimePlugin::getTabLabel()
{
    return tr("Time");
}
