#include "multieditordate.h"
#include <QDateEdit>

QStringList MultiEditorDate::formats;

MultiEditorDate::MultiEditorDate(QWidget* parent)
    : MultiEditorDateTime(parent)
{
    setDisplayFormat(formats.first());
}

void MultiEditorDate::staticInit()
{
    formats << "yyyy-MM-dd";
}

QStringList MultiEditorDate::getParsingFormats()
{
    return MultiEditorDateTime::getParsingFormats();
}


MultiEditorWidget*MultiEditorDatePlugin::getInstance()
{
    return new MultiEditorDate();
}

bool MultiEditorDatePlugin::validFor(const DataType& dataType)
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
        case DataType::DATETIME:
        case DataType::TIME:
        case DataType::ANY:
        case DataType::unknown:
            break;
        case DataType::DATE:
            return true;
    }
    return false;
}

int MultiEditorDatePlugin::getPriority(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
        case DataType::BOOLEAN:
        case DataType::ANY:
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
        case DataType::DATETIME:
        case DataType::unknown:
            break;
        case DataType::DATE:
            return 1;
    }
    return 10;
}

QString MultiEditorDatePlugin::getTabLabel()
{
    return tr("Date");
}
