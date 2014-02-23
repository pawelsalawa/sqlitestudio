#ifndef MULTIEDITORTEXT_H
#define MULTIEDITORTEXT_H

#include "multieditorwidget.h"
#include "common/extactioncontainer.h"

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

#endif // MULTIEDITORTEXT_H
