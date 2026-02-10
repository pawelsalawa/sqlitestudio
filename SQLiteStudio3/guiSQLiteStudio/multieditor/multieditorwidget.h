#ifndef MULTIEDITORWIDGET_H
#define MULTIEDITORWIDGET_H

#include "guiSQLiteStudio_global.h"
#include <QWidget>

class GUI_API_EXPORT MultiEditorWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit MultiEditorWidget(QWidget *parent = nullptr);

        virtual void setValue(const QVariant& value) = 0;
        virtual QVariant getValue() = 0;
        virtual void setReadOnly(bool value) = 0;
        virtual QList<QWidget*> getNoScrollWidgets() = 0;
        virtual void focusThisWidget() = 0;
        virtual QString getPreferredFileFilter();

        void installEventFilter(QObject* filterObj);

        void setTabLabel(const QString& value);
        QString getTabLabel();
        bool isUpToDate() const;
        void setUpToDate(bool value);
        void notifyAboutUnload();

    private:
        bool upToDate = true;
        QString tabLabel;

    signals:
        void valueModified();
        void aboutToBeDeleted();
};

#endif // MULTIEDITORWIDGET_H
