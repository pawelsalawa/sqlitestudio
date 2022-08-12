#ifndef COMPLETERWINDOW_H
#define COMPLETERWINDOW_H

#include "expectedtoken.h"
#include "completionhelper.h"
#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QModelIndex>

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
        QString getSnippetName() const;

    protected:
        void changeEvent(QEvent *e);
        void keyPressEvent(QKeyEvent* e);
        void showEvent(QShowEvent* e);

    private:
        void updateCurrent();
        QString getStatusMsg(const QModelIndex& index);
        void updateFilter();
        void init();
        void refreshSnippets();

        Ui::CompleterWindow *ui = nullptr;
        CompleterModel* model = nullptr;
        SqlEditor* sqlEditor = nullptr;
        QString filter;
        Db* db = nullptr;
        bool wrappedFilter = false;
        QShortcut* modeChangeShortcut = nullptr;
        QList<QShortcut*> snippetShortcuts;
        QSignalMapper* snippetSignalMapper = nullptr;

    private slots:
        void focusOut();
        void doubleClicked(const QModelIndex& index);
        void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
        void modeChangeRequested();
        void snippetHotkeyPressed(int index);
        void snippetDoubleClicked(QListWidgetItem* item);

    signals:
        void textTyped(const QString& text);
        void backspacePressed();
        void leftPressed();
        void rightPressed();
};

#endif // COMPLETERWINDOW_H
