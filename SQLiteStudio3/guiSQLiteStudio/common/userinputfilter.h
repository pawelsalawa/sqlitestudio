#ifndef USERINPUTFILTER_H
#define USERINPUTFILTER_H

#include "guiSQLiteStudio_global.h"
#include <QObject>

class QLineEdit;
class LazyTrigger;

class GUI_API_EXPORT UserInputFilter : public QObject
{
        Q_OBJECT

    public:
        /**
         * @param onlyEdited If true, the #textEdited() signal will be used, instead of #textChanged(),
         * so that only manual editions trigger the filter, while programmating changes do not.
         */
        UserInputFilter(QLineEdit* lineEdit, QObject* filterHandler, const char* handlerSlot, bool onlyEdited = false);

        void setDelay(int msecs);

    private:
        LazyTrigger* trigger = nullptr;
        QLineEdit* lineEdit = nullptr;

    private slots:
        void filterModified();
        void applyFilter();

    signals:
        void applyFilter(const QString& value);
};

#endif // USERINPUTFILTER_H
