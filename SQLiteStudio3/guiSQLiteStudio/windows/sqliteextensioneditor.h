#ifndef SQLITEEXTENSIONEDITOR_H
#define SQLITEEXTENSIONEDITOR_H

#include "icon.h"
#include "mdichild.h"
#include <QItemSelection>
#include <QWidget>

namespace Ui {
    class SqliteExtensionEditor;
}

class QToolBar;
class SqliteExtensionEditorModel;
class QSortFilterProxyModel;
class SelectableDbModel;
class Db;
class LazyTrigger;

CFG_KEY_LIST(SqliteExtensionEditor, QObject::tr("A SQLite extension editor window"),
    CFG_KEY_ENTRY(COMMIT,     QKeySequence::Save,        QObject::tr("Commit the pending changes"))
    CFG_KEY_ENTRY(ROLLBACK,   QKeySequence::Cancel,      QObject::tr("Rollback the pending changes"))
)

class SqliteExtensionEditor : public MdiChild
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
        Q_ENUM(Action)

        enum ToolBar
        {
            TOOLBAR
        };

        explicit SqliteExtensionEditor(QWidget *parent = nullptr);
        ~SqliteExtensionEditor();

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
        int getCurrentExtensionRow() const;
        void extensionDeselected(int row);
        void extensionSelected(int row);
        void clearEdits();
        void selectExtension(int row);
        QStringList getCurrentDatabases() const;
        bool tryToLoad(const QString& filePath, const QString& initFunc, QString* resultError);
        bool validateExtension(bool* fileOk = nullptr,
                               bool* initOk = nullptr,
                               QString* fileError = nullptr);
        bool validateExtension(int row);
        bool validateCurrentExtension();
        bool validateExtension(const QString& filePath,
                               const QString& initFunc,
                               bool* fileOk = nullptr,
                               bool* initOk = nullptr,
                               QString* fileError = nullptr);
        void initStateForAll();

        Ui::SqliteExtensionEditor *ui;
        SqliteExtensionEditorModel* model = nullptr;
        QSortFilterProxyModel* extensionFilterModel = nullptr;
        SelectableDbModel* dbListModel = nullptr;
        bool currentModified = false;
        bool updatesForSelection = false;
        Db* probingDb = nullptr;
        LazyTrigger* statusUpdateTrigger = nullptr;
        bool nameGenerationActive = true;

    private slots:
        void help();
        void commit();
        void rollback();
        void newExtension();
        void deleteExtension();
        void updateState();
        void updateCurrentExtensionState();
        void extensionSelected(const QItemSelection& selected, const QItemSelection& deselected);
        void updateModified();
        void generateName();
        void applyFilter(const QString& value);
        void browseForFile();
};

#endif // SQLITEEXTENSIONEDITOR_H
