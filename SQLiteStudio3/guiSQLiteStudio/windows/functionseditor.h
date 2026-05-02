#ifndef FUNCTIONSEDITOR_H
#define FUNCTIONSEDITOR_H

#include "mdichild.h"
#include "common/extactioncontainer.h"
#include "services/functionmanager.h"
#include <QItemSelection>
#include <QSortFilterProxyModel>


class QPlainTextEdit;
namespace Ui {
class FunctionsEditor;
}

class FunctionsEditorModel;
class ScriptingPlugin;
class SyntaxHighlighterPlugin;
class DbTreeItem;
class QTreeWidgetItem;
class QSyntaxHighlighter;
class SelectableDbModel;

CFG_KEY_LIST(FunctionsEditor, QObject::tr("A function editor window"),
    CFG_KEY_ENTRY(COMMIT,     QKeySequence::Save,        QObject::tr("Commit the pending changes"))
    CFG_KEY_ENTRY(ROLLBACK,   QKeySequence::Cancel,      QObject::tr("Rollback the pending changes"))
)

class GUI_API_EXPORT FunctionsEditor : public MdiChild
{
    Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK,
            ADD,
            DELETE,
            ARG_ADD,
            ARG_EDIT,
            ARG_DEL,
            ARG_MOVE_UP,
            ARG_MOVE_DOWN,
            IMPORT,
            EXPORT,
            HELP
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR
        };

        explicit FunctionsEditor(QWidget *parent = 0);
        ~FunctionsEditor();

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
        enum CodeTab
        {
            SCALAR,
            INIT,
            STEP,
            INVERSE,
            FINAL
        };

        void init();
        void initCodeTabs();
        void setupContextMenu();
        int getCurrentFunctionRow() const;
        void functionDeselected(int srcRow);
        void functionSelected(int srcRow);
        void clearEdits();
        void selectFunction(int srcRow);
        void setFont(const QFont& font);
        QModelIndex fnRowToSrc(const QModelIndex& idx) const;
        QModelIndex getSelectedArg() const;
        QStringList getCurrentArgList() const;
        QStringList getCurrentDatabases() const;
        FunctionManager::ScriptFunction::Type getCurrentFunctionType() const;
        void safeClearHighlighter(QSyntaxHighlighter*& highlighterPtr);

        Ui::FunctionsEditor *ui = nullptr;
        FunctionsEditorModel* model = nullptr;
        QSortFilterProxyModel* functionFilterModel = nullptr;
        bool currentModified = false;
        QHash<QString,ScriptingPlugin*> scriptingPlugins;
        QHash<QString,SyntaxHighlighterPlugin*> highlighterPlugins;
        SelectableDbModel* dbListModel = nullptr;
        QString currentHighlighterLang;
        QSyntaxHighlighter* currentScalarHighlighter = nullptr;
        QSyntaxHighlighter* currentStepHighlighter = nullptr;
        QSyntaxHighlighter* currentFinalHighlighter = nullptr;
        QSyntaxHighlighter* currentInitHighlighter = nullptr;
        QSyntaxHighlighter* currentInverseHighlighter = nullptr;
        bool updatesForSelection = false;
        QHash<CodeTab, int> tabIdx;

    private slots:
        void commit();
        void rollback();
        void newFunction();
        void deleteFunction();
        void updateModified();
        void updateState();
        void updateCurrentFunctionState();
        void functionSelected(const QItemSelection& selected, const QItemSelection& deselected);
        void addFunctionArg();
        void editFunctionArg();
        void delFunctionArg();
        void moveFunctionArgUp();
        void moveFunctionArgDown();
        bool updateArgsState();
        void applyFilter(const QString& value);
        void help();
        void changeFont(const QVariant& font);
        void cfgFunctionListChanged();
        void importFunctions();
        void exportFunctions();
};

#endif // FUNCTIONSEDITOR_H
