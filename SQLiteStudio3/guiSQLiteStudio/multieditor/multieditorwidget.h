#ifndef MULTIEDITORWIDGET_H
#define MULTIEDITORWIDGET_H

#include "guiSQLiteStudio_global.h"
#include "searchtextlocator.h"
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

        /**
         * @brief Provides text locator bound to this editor widget.
         * @return Locator instance, or null if text searching is not supported by this editor widget.
         *
         * By default text searching is not supported.
         * Create the text locator in the editor widget, then return it from this method
         * to make the editor enabled for Search Dialog, usually triggered with Ctrl+f/Cmd+f hotkey.
         *
         * The locator object returned is still owned by the editor widget and it's the widget's responsibility
         * to delete it, although it's usually handled by the parent-child relation of
         * the QPlainTextEdit and the SearchTextLocator.
         */
        virtual SearchTextLocator* getTextLocator();

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

        /**
         * This signal can be emitted by the editor widget to explicitly request opening the dialog.
         * By default this dialog is requested by Ctrl+f/Cmd+f, but the custom editor widget
         * may have its own toolbar button to trigger the dialog.
         */
        void requestFindDialog();
};

#endif // MULTIEDITORWIDGET_H
