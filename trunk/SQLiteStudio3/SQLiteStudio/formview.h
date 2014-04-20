#ifndef FORMVIEW_H
#define FORMVIEW_H

#include "datagrid/sqlquerymodelcolumn.h"
#include "multieditor/multieditor.h"
#include <QWidget>
#include <QPointer>
#include <QScrollArea>

class SqlQueryModel;
class SqlQueryView;
class QDataWidgetMapper;

class FormView : public QScrollArea
{
    Q_OBJECT

    public:
        explicit FormView(QWidget *parent = 0);

        void init();

        SqlQueryModel* getModel() const;
        void setModel(SqlQueryModel* value);

        bool isModified() const;

        SqlQueryView* getGridView() const;
        void setGridView(SqlQueryView* value);

        int getCurrentRow();

    private:
        void reloadInternal();
        void addColumn(int colIdx, const QString& name, const SqlQueryModelColumn::DataType& dataType, bool readOnly);
        bool isCurrentRowModifiedInGrid();
        void updateDeletedState();

        static const int margins = 2;
        static const int spacing = 2;
        static const int minimumFieldHeight = 40;

        QDataWidgetMapper* dataMapper;
        QPointer<SqlQueryView> gridView;
        QPointer<SqlQueryModel> model;
        QWidget* contents;
        QList<QWidget*> widgets;
        QList<MultiEditor*> editors;
        QList<bool> readOnly;
        bool valueModified = false;
        bool currentIndexUpdating = false;

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

    signals:
        void commitStatusChanged();
        void currentRowChanged();
};

#endif // FORMVIEW_H
