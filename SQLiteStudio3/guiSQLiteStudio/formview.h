#ifndef FORMVIEW_H
#define FORMVIEW_H

#include "guiSQLiteStudio_global.h"
#include "multieditor/multieditor.h"
#include "common/extactioncontainer.h"
#include <QWidget>
#include <QPointer>
#include <QScrollArea>

class SqlQueryModel;
class SqlQueryView;
class DataWidgetMapper;

CFG_KEY_LIST(FormView, QObject::tr("Data form view"),
    CFG_KEY_ENTRY(COMMIT,            Qt::CTRL | Qt::Key_Return,               QObject::tr("Commit changes for current row"))
    CFG_KEY_ENTRY(ROLLBACK,          Qt::ALT | Qt::SHIFT | Qt::Key_Backspace, QObject::tr("Rollback changes for current row"))
    CFG_KEY_ENTRY(FIRST_ROW,         Qt::CTRL | Qt::ALT | Qt::Key_PageUp,     QObject::tr("Go to first row on current page"))
    CFG_KEY_ENTRY(NEXT_ROW,          Qt::CTRL | Qt::ALT | Qt::Key_Right,      QObject::tr("Go to next row"))
    CFG_KEY_ENTRY(PREV_ROW,          Qt::CTRL | Qt::ALT | Qt::Key_Left,       QObject::tr("Go to previous row"))
    CFG_KEY_ENTRY(LAST_ROW,          Qt::CTRL | Qt::ALT | Qt::Key_PageDown,   QObject::tr("Go to last row on current page"))
    CFG_KEY_ENTRY(INSERT_ROW,        Qt::Key_Insert,                          QObject::tr("Insert new row"))
    CFG_KEY_ENTRY(DELETE_ROW,        Qt::CTRL | Qt::Key_Delete,               QObject::tr("Delete current row"))
)

class GUI_API_EXPORT FormView : public QScrollArea, public ExtActionContainer
{
    Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK,
            FIRST_ROW,
            NEXT_ROW,
            PREV_ROW,
            LAST_ROW,
            INSERT_ROW,
            DELETE_ROW
        };
        Q_ENUM(Action)

        enum ToolBar
        {
        };

        explicit FormView(QWidget *parent = 0);

        void init();

        SqlQueryModel* getModel() const;
        void setModel(SqlQueryModel* value);

        bool isModified() const;

        SqlQueryView* getGridView() const;
        void setGridView(SqlQueryView* value);

        int getCurrentRow();

    protected:
        void createActions();
        void setupDefShortcuts();
        QToolBar* getToolBar(int toolbar) const;
        void showEvent(QShowEvent* event);

    private:
        void reloadInternal();
        MultiEditor* addColumn(int colIdx, SqlQueryModelColumn* column);
        bool isCurrentRowModifiedInGrid();
        void updateDeletedState();

        static const int margins = 2;
        static const int spacing = 2;
        static const int minimumFieldHeight = 40;

        DataWidgetMapper* dataMapper = nullptr;
        QPointer<SqlQueryView> gridView;
        QPointer<SqlQueryModel> model;
        QWidget* contents = nullptr;
        QList<QWidget*> widgets;
        QList<MultiEditor*> editors;
        QList<bool> readOnly;
        bool valueModified = false;
        bool currentIndexUpdating = false;
        bool shouldReload = false;
        int indexForReload = 0;

    private slots:
        void dataLoaded(bool successful);
        void currentIndexChanged(int index);
        void editorValueModified();
        void gridCommitRollbackStatusChanged();

    public slots:
        void copyDataToGrid();
        void updateFromGrid();
        void load();
        void reload();
        void focusFirstEditor();

    signals:
        void commitStatusChanged();
        void currentRowChanged();
        void requestForCommit();
        void requestForRollback();
        void requestForNextRow();
        void requestForPrevRow();
        void requestForFirstRow();
        void requestForLastRow();
        void requestForRowInsert();
        void requestForRowDelete();
};

#endif // FORMVIEW_H
