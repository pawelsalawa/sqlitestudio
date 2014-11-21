#ifndef MULTIEDITORWIDGET_H
#define MULTIEDITORWIDGET_H

#include "guiSQLiteStudio_global.h"
#include <QWidget>

class GUI_API_EXPORT MultiEditorWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit MultiEditorWidget(QWidget *parent = 0);

        virtual void setValue(const QVariant& value) = 0;
        virtual QVariant getValue() = 0;
        virtual void setReadOnly(bool value) = 0;
        virtual QList<QWidget*> getNoScrollWidgets() = 0;
        virtual QString getTabLabel() = 0;
        virtual void focusThisWidget() = 0;

        void installEventFilter(QObject* filterObj);

        bool isUpToDate() const;
        void setUpToDate(bool value);

    private:
        bool upToDate = true;

    signals:
        void valueModified();
};

#endif // MULTIEDITORWIDGET_H
