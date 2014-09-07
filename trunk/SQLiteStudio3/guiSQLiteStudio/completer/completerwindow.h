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

class CompleterModel;
class QSizeGrip;
class SqlEditor;

class GUI_API_EXPORT CompleterWindow : public QDialog
{
        Q_OBJECT

    public:
        explicit CompleterWindow(SqlEditor* parent = 0);
        ~CompleterWindow();

        void reset();
        void setData(const CompletionHelper::Results& completionResults);
        void setDb(Db* db);
        ExpectedTokenPtr getSelected();
        int getNumberOfCharsToRemove();
        void shringFilterBy(int chars);
        void extendFilterBy(const QString& text);
        bool immediateResolution();

    protected:
        void changeEvent(QEvent *e);
        void keyPressEvent(QKeyEvent* e);

    private:
        void updateCurrent();
        QString getStatusMsg(const QModelIndex& index);
        void updateFilter();
        void init();

        Ui::CompleterWindow *ui;
        CompleterModel* model;
        SqlEditor* sqlEditor;
        QString filter;
        Db* db = nullptr;
        bool wrappedFilter = false;

    private slots:
        void focusOut();
        void doubleClicked(const QModelIndex& index);
        void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    signals:
        void textTyped(const QString& text);
        void backspacePressed();
        void leftPressed();
        void rightPressed();
};

#endif // COMPLETERWINDOW_H
