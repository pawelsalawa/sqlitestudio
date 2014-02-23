#ifndef MULTIEDITORDATETIME_H
#define MULTIEDITORDATETIME_H

#include "multieditorwidget.h"
#include <QStringList>
#include <QDateTime>

class QCalendarWidget;
class QDateTimeEdit;
class QLabel;

class MultiEditorDateTime : public MultiEditorWidget
{
        Q_OBJECT
    public:
        explicit MultiEditorDateTime(QWidget *parent = 0);

        static void staticInit();

        void setValue(const QVariant& value);
        QVariant getValue();
        bool needsValueUpdate();

        QList<QWidget*> getNoScrollWidgets();

        bool getReadOnly() const;
        void setReadOnly(bool value);

    protected:
        void updateCalendarDisplay();
        void setDisplayFormat(const QString& format);

        virtual QStringList getParsingFormats();

        QDateTimeEdit* dateTimeEdit;
        bool showCalendars = true;

    private:
        enum FormatType
        {
            STRING,
            JULIAN_DAY,
            OTHER
        };

        void updateReadOnlyDisplay();
        QDateTime fromString(const QString& value);

        static QStringList formats;

        QLabel* dateTimeLabel;
        QCalendarWidget* calendar;
        QString originalValueFormat;
        FormatType formatType;
        bool updatingCalendar = false;
        bool readOnly = false;

    private slots:
        void calendarDateChanged();
        void dateChanged(const QDate& date);
        void timeChanged(const QTime& time);
};

#endif // MULTIEDITORDATETIME_H
