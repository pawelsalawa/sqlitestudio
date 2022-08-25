#ifndef MULTIEDITORTEXT_H
#define MULTIEDITORTEXT_H

#include "multieditorwidget.h"
#include "multieditorwidgetplugin.h"
#include "common/extactioncontainer.h"
#include "plugins/builtinplugin.h"

class QPlainTextEdit;
class QMenu;

CFG_KEY_LIST(MultiEditorText, QObject::tr("Cell text value editor"),
     CFG_KEY_ENTRY(CUT,     QKeySequence::Cut,      QObject::tr("Cut selected text"))
     CFG_KEY_ENTRY(COPY,    QKeySequence::Copy,     QObject::tr("Copy selected text"))
     CFG_KEY_ENTRY(PASTE,   QKeySequence::Paste,    QObject::tr("Paste from clipboard"))
     CFG_KEY_ENTRY(DELETE,  QKeySequence::Delete,   QObject::tr("Delete selected text"))
     CFG_KEY_ENTRY(UNDO,    QKeySequence::Undo,     QObject::tr("Undo"))
     CFG_KEY_ENTRY(REDO,    QKeySequence::Redo,     QObject::tr("Redo"))
)

class GUI_API_EXPORT MultiEditorText : public MultiEditorWidget, public ExtActionContainer
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
        Q_ENUM(Action)

        enum ToolBar
        {
        };

        explicit MultiEditorText(QWidget *parent = 0);

        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool value);
        QToolBar* getToolBar(int toolbar) const;
        void focusThisWidget();
        QList<QWidget*> getNoScrollWidgets();

    protected:
        void createActions();
        void setupDefShortcuts();

    private:
        void setupMenu();

        QPlainTextEdit* textEdit = nullptr;
        QMenu* contextMenu = nullptr;

    private slots:
        void modificationChanged(bool changed);
        void deleteSelected();
        void showCustomMenu(const QPoint& point);
        void updateUndoAction(bool enabled);
        void updateRedoAction(bool enabled);
        void updateCopyAction(bool enabled);
        void toggleTabFocus();
};

class GUI_API_EXPORT MultiEditorTextPlugin : public BuiltInPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")
    SQLITESTUDIO_PLUGIN_DESC("Standard text data editor.")
    SQLITESTUDIO_PLUGIN_TITLE("Text")
    SQLITESTUDIO_PLUGIN_VERSION(10000)

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const DataType& dataType);
        int getPriority(const DataType& dataType);
        QString getTabLabel();
};

#endif // MULTIEDITORTEXT_H
