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
