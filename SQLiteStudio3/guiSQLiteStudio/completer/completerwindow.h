#ifndef COMPLETERWINDOW_H
#define COMPLETERWINDOW_H

#include "expectedtoken.h"
#include "completionhelper.h"
#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QModelIndex>


class CompleterSnippetDelegate;

class QLineEdit;
namespace Ui {
    class CompleterWindow;
}
class QListWidgetItem;
class QSignalMapper;
class CompleterModel;
class QSizeGrip;
class SqlEditor;
class QShortcut;

class GUI_API_EXPORT CompleterWindow : public QDialog
{
        Q_OBJECT

    public:
        enum Mode
        {
            CODE,
            SNIPPETS
        };

        enum SnippetKeyMode
        {
            HOTKEY,
            FILTER
        };

        explicit CompleterWindow(SqlEditor* parent = 0);
        ~CompleterWindow();

        void reset();
        void setData(const CompletionHelper::Results& completionResults);
        void setDb(Db* db);
        ExpectedTokenPtr getSelected() const;
        int getNumberOfCharsToRemove();
        void shringFilterBy(int chars);
        void extendFilterBy(const QString& text);
        bool immediateResolution();
        Mode getMode() const;
        SnippetKeyMode getSnippetKeyMode() const;
        QString getSnippetName() const;
        void setInitialMode(Mode newInitialMode);

    protected:
        void changeEvent(QEvent *e) override;
        void keyPressEvent(QKeyEvent* e) override;
        void showEvent(QShowEvent* e) override;
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        void updateCurrent();
        QString getStatusMsg(const QModelIndex& index);
        bool hasVisibleSnippets() const;
        void applyFilterToSnippets();
        void updateFilter();
        void init();
        void refreshSnippets();
        void setMode(Mode mode);
        QString getSnippetsStatusMsg() const;
        void setSnippetsKeyMode(SnippetKeyMode mode);

        Ui::CompleterWindow *ui = nullptr;
        CompleterModel* model = nullptr;
        SqlEditor* sqlEditor = nullptr;
        QString filter;
        Db* db = nullptr;
        bool wrappedFilter = false;
        Mode initialMode = CODE;
        SnippetKeyMode snippetKeyMode = HOTKEY;
        QShortcut* modeChangeShortcut = nullptr;
        QList<QShortcut*> snippetShortcuts;
        QSignalMapper* snippetSignalMapper = nullptr;
        CompleterSnippetDelegate* snippetDelegate = nullptr;

    private slots:
        void focusOut();
        void doubleClicked(const QModelIndex& index);
        void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
        void modeChangeRequested();
        void snippetHotkeyPressed(int index);
        void snippetDoubleClicked(QListWidgetItem* item);
        void toggleSnippetsKeyMode();

    signals:
        void textTyped(const QString& text);
        void backspacePressed();
        void leftPressed();
        void rightPressed();
};

#endif // COMPLETERWINDOW_H
