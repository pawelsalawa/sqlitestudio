#ifndef MULTIEDITORTEXT_H
#define MULTIEDITORTEXT_H

#include "multieditorwidget.h"
#include "multieditorwidgetplugin.h"
#include "common/extactioncontainer.h"
#include "plugins/genericplugin.h"

class QPlainTextEdit;
class QMenu;

class MultiEditorText : public MultiEditorWidget, public ExtActionContainer
{
        Q_OBJECT
    public:
        enum Action
        {
            TAB_CHANGES_FOCUS,
            CUT,
            COPY,
            PASTE,
            DELETE,
            UNDO,
            REDO
        };

        explicit MultiEditorText(QWidget *parent = 0);

        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool value);
        QString getTabLabel();

        QList<QWidget*> getNoScrollWidgets();

    protected:
        void createActions();
        void setupDefShortcuts();

    private:
        void setupMenu();

        QPlainTextEdit* textEdit;
        QMenu* contextMenu;

    private slots:
        void modificationChanged(bool changed);
        void deleteSelected();
        void showCustomMenu(const QPoint& point);
        void updateUndoAction(bool enabled);
        void updateRedoAction(bool enabled);
        void updateCopyAction(bool enabled);
        void toggleTabFocus();
};

class MultiEditorTextPlugin : public GenericPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")
    SQLITESTUDIO_PLUGIN_DESC("Standard text data editor.")
    SQLITESTUDIO_PLUGIN_TITLE("Text")
    SQLITESTUDIO_PLUGIN_VERSION(10000)

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const SqlQueryModelColumn::DataType& dataType);
        int getPriority(const SqlQueryModelColumn::DataType& dataType);
};

#endif // MULTIEDITORTEXT_H
