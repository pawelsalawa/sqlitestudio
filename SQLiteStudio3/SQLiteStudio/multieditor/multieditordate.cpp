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
