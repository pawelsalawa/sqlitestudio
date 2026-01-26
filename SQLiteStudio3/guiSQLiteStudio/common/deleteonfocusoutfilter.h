#ifndef DELETEONFOCUSOUTFILTER_H
#define DELETEONFOCUSOUTFILTER_H

#include "guiSQLiteStudio_global.h"
#include <QObject>
#include <QSet>

class GUI_API_EXPORT DeleteOnFocusOutFilter : public QObject
{
    Q_OBJECT

    public:
        using QObject::QObject;

        void ignoredReason(Qt::FocusReason reason);

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        QSet<Qt::FocusReason> ignoredFocusReasons;

    signals:
        void aboutToDelete(QObject* object, Qt::FocusReason reason);
};

#endif // DELETEONFOCUSOUTFILTER_H
