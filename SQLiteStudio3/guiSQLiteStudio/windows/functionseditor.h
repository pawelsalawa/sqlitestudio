#ifndef FUNCTIONSEDITOR_H
#define FUNCTIONSEDITOR_H

#include "mdichild.h"
#include "common/extactioncontainer.h"
#include "services/functionmanager.h"
#include <QItemSelection>
#include <QSortFilterProxyModel>

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
            HELP
        };
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR
        };

        explicit FunctionsEditor(QWidget *parent = 0);
        ~FunctionsEditor();

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
        int getCurrentFunctionRow() const;
        void functionDeselected(int row);
        void functionSelected(int row);
        void clearEdits();
        void selectFunction(int row);
        void setFont(const QFont& font);
        QModelIndex getSelectedArg() const;
        QStringList getCurrentArgList() const;
        QStringList getCurrentDatabases() const;
        FunctionManager::ScriptFunction::Type getCurrentFunctionType() const;

        Ui::FunctionsEditor *ui = nullptr;
        FunctionsEditorModel* model = nullptr;
        QSortFilterProxyModel* functionFilterModel = nullptr;
        bool currentModified = false;
        QHash<QString,ScriptingPlugin*> scriptingPlugins;
        QHash<QString,SyntaxHighlighterPlugin*> highlighterPlugins;
        SelectableDbModel* dbListModel = nullptr;
        QString currentHighlighterLang;
        QSyntaxHighlighter* currentMainHighlighter = nullptr;
        QSyntaxHighlighter* currentFinalHighlighter = nullptr;
        QSyntaxHighlighter* currentInitHighlighter = nullptr;
        bool updatesForSelection = false;

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
};

#endif // FUNCTIONSEDITOR_H
