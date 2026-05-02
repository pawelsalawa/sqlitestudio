#ifndef CODESNIPPETEDITOR_H
#define CODESNIPPETEDITOR_H

#include "guiSQLiteStudio_global.h"
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

class GUI_API_EXPORT CodeSnippetEditor : public MdiChild
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
            IMPORT,
            EXPORT,
            HELP
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR
        };

        explicit CodeSnippetEditor(QWidget *parent = nullptr);
        ~CodeSnippetEditor();

        bool restoreSessionNextTime() override;
        bool isUncommitted() const override;
        QString getQuitUncommittedConfirmMessage() const override;

    protected:
        QVariant saveSession() override;
        bool restoreSession(const QVariant &sessionValue) override;
        Icon* getIconNameForMdiWindow() override;
        QString getTitleForMdiWindow() override;
        void createActions() override;
        void setupDefShortcuts() override;
        QToolBar* getToolBar(int toolbar) const override;

    private:
        void init();
        void setupContextMenu();
        QModelIndex getCurrentSnippetIndex() const;
        void snippetDeselected(const QModelIndex& srcRow);
        void snippetSelected(const QModelIndex& idx);
        void selectSnippet(const QModelIndex& idx, bool forRowMovement = false);
        void clearEdits();

        Ui::CodeSnippetEditor *ui;
        CodeSnippetEditorModel* dataModel = nullptr;
        QSortFilterProxyModel* viewModel = nullptr;
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
        void cfgCodeSnippetListChanged();
        void importSnippets();
        void exportSnippets();

    public slots:
        void editSnippet(const QString& name);
};

#endif // CODESNIPPETEDITOR_H
