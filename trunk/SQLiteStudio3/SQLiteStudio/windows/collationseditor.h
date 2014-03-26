#ifndef COLLATIONSEDITOR_H
#define COLLATIONSEDITOR_H

#include "mdichild.h"
#include "common/extactioncontainer.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QWidget>

namespace Ui {
    class CollationsEditor;
}

class SyntaxHighlighterPlugin;
class SelectableDbModel;
class CollationsEditorModel;
class QSortFilterProxyModel;
class QSyntaxHighlighter;

class CollationsEditor : public MdiChild, public ExtActionContainer
{
        Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK,
            ADD,
            DELETE,
            HELP
        };

        explicit CollationsEditor(QWidget *parent = 0);
        ~CollationsEditor();

        bool restoreSessionNextTime();

    protected:
        QVariant saveSession();
        bool restoreSession(const QVariant &sessionValue);
        QString getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();

    private:
        void init();
        int getCurrentCollationRow() const;
        void collationDeselected(int row);
        void collationSelected(int row);
        void clearEdits();
        void selectCollation(int row);
        QStringList getCurrentDatabases() const;

        Ui::CollationsEditor *ui;
        CollationsEditorModel* model = nullptr;
        QSortFilterProxyModel* collationFilterModel = nullptr;
        SelectableDbModel* dbListModel = nullptr;
        QHash<QString,SyntaxHighlighterPlugin*> highlighterPlugins;
        QSyntaxHighlighter* currentHighlighter = nullptr;
        QString currentHighlighterLang;
        bool currentModified = false;

    private slots:
        void help();
        void commit();
        void rollback();
        void newCollation();
        void deleteCollation();
        void updateState();
        void updateCurrentCollationState();
        void collationSelected(const QItemSelection& selected, const QItemSelection& deselected);
        void validateName();
        void updateModified();
        void applyFilter(const QString& value);
};

#endif // COLLATIONSEDITOR_H
