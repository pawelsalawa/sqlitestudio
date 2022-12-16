#ifndef CODESNIPPETEDITOR_H
#define CODESNIPPETEDITOR_H

#include "mdichild.h"
#include <QWidget>

namespace Ui {
class CodeSnippetEditor;
}

class CodeSnippetEditorModel;
class QSortFilterProxyModel;
class QSyntaxHighlighter;
class QItemSelection;

CFG_KEY_LIST(CodeSnippetEditor, QObject::tr("A code snippets editor window"),
    CFG_KEY_ENTRY(COMMIT,     QKeySequence::Save,        QObject::tr("Commit the pending changes"))
    CFG_KEY_ENTRY(ROLLBACK,   QKeySequence::Cancel,      QObject::tr("Rollback the pending changes"))
)

class CodeSnippetEditor : public MdiChild
{
    Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK,
            ADD,
            DELETE,
            MOVE_UP,
            MOVE_DOWN,
            HELP
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR
        };

        explicit CodeSnippetEditor(QWidget *parent = nullptr);
        ~CodeSnippetEditor();

        bool restoreSessionNextTime();
        bool isUncommitted() const;
        QString getQuitUncommittedConfirmMessage() const;

    protected:
        QVariant saveSession();
        bool restoreSession(const QVariant &sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();
        QToolBar* getToolBar(int toolbar) const;

    private:
        void init();
        int getCurrentSnippetRow() const;
        void snippetDeselected(int row);
        void snippetSelected(int row);
        void selectSnippet(int row, bool forRowMovement = false);
        void clearEdits();

        Ui::CodeSnippetEditor *ui;
        CodeSnippetEditorModel* model = nullptr;
        QSortFilterProxyModel* snippetFilterModel = nullptr;
        bool currentModified = false;
        bool updatesForSelection = false;
        bool skipModelUpdatesDuringSelection = false;

    private slots:
        void commit();
        void rollback();
        void newSnippet();
        void deleteSnippet();
        void moveSnippetUp();
        void moveSnippetDown();
        void updateModified();
        void updateCurrentSnippetState();
        void updateState();
        void snippetSelected(const QItemSelection& selected, const QItemSelection& deselected);
        void applyFilter(const QString& value);
        void changeFont(const QVariant& font);
        void clearAssistantShortcutPressed();
        void help();
};

#endif // CODESNIPPETEDITOR_H
